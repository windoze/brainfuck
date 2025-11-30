//
//  bfcodegen.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

// Upgrade to LLVM 18
// by 星灿长风v(StarWindv) on 2025/11/29


#include "bfcodegen.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>

using namespace llvm;
namespace brainfuck {
    namespace details {
        
        inline ConstantInt *const_int(LLVMContext &ctx, IntegerType *t, int64_t n=0, bool is_signed=false) {
            return ConstantInt::get(t, n, is_signed);
        }
        
        struct context {
            context(Module &m, unsigned int cell_size, size_t storage_size)
            : module(m)
            , ctx(module.getContext())
            , builder(ctx)
            , CellType(IntegerType::get(ctx, cell_size))
            , StorageType(ArrayType::get(CellType, storage_size))
            , SPType(IntegerType::get(ctx, sizeof(storage_size)*8))
            , get_char_(0)
            , put_char_(0)
            , entry_(0)
            {
                // Initialize storage
                storage = new GlobalVariable(m,
                                             StorageType,
                                             false,
                                             GlobalValue::InternalLinkage,
                                             ConstantAggregateZero::get(StorageType),
                                             "s");
                
                // Initialize sp
                sp = new GlobalVariable(m,
                                        SPType,
                                        false,
                                        GlobalValue::InternalLinkage,
                                        const_int(ctx, SPType, 0),
                                        "sp");
                
                // Initialize external functions
                {
                    FunctionType *FT = FunctionType::get(IntegerType::getInt32Ty(ctx), false);
                    get_char_ = Function::Create(FT, Function::ExternalLinkage, "getchar", &module);
                }
                {
                    std::vector<Type *> args(1, IntegerType::getInt32Ty(ctx));
                    FunctionType *FT = FunctionType::get(Type::getVoidTy(ctx), args, false);
                    put_char_ = Function::Create(FT, Function::ExternalLinkage, "putchar", &module);
                }
                
                // Initialize entry point, which is a function with name 'main'
                {
                    FunctionType *MainType = FunctionType::get(Type::getVoidTy(ctx), false);
                    entry_ = Function::Create(MainType, Function::ExternalLinkage, "main", &m);
                    BasicBlock *BB = BasicBlock::Create(ctx, "", entry_);
                    builder.SetInsertPoint(BB);
                }
            }
            
            ~context() {
                // Close up function 'main'
                builder.CreateRetVoid();
                llvm::verifyFunction(*entry_);
            }
            
            /// Return pointer to current cell
            Value *current() {
                std::vector<Value *> idx;
                idx.push_back(const_int(ctx, IntegerType::getInt64Ty(ctx), 0));
                idx.push_back(builder.CreateLoad(SPType, sp, "sp_load"));
                return builder.CreateGEP(StorageType, storage, idx, "ptr");
            }
            
            Instruction *get_char() {
                Value *result = builder.CreateCall(get_char_);
                // Truncate to cell size
                return cast<Instruction>(builder.CreateTrunc(result, CellType, "getchar_trunc"));
            }

            Instruction *put_char(Value *arg) {
                // Extend to 32 bits for putchar
                Value *extended = builder.CreateZExt(arg, IntegerType::getInt32Ty(ctx), "putchar_ext");
                return builder.CreateCall(put_char_, extended);
            }
            
            Module &module;
            LLVMContext &ctx;
            IRBuilder<> builder;
            
            // Types
            IntegerType *CellType;
            ArrayType *StorageType;
            IntegerType *SPType;
            
            // Global Variables
            GlobalVariable *storage;
            GlobalVariable *sp;
            
            // Predefined Functions
            Function *get_char_;
            Function *put_char_;
            
            // Entry Point
            Function *entry_;
        };  // End of context
        
        struct codegen_visitor : public boost::static_visitor<void> {
            codegen_visitor(context &ctx) : ctx_(ctx) {}
            
            template<typename T>
            void operator()(const T &n) const {
                codegen(n);
            }
            
            void codegen(const ast::MoveLeft &n) const {
                // sp -= n.count_
                Value *sp_val = ctx_.builder.CreateLoad(ctx_.SPType, ctx_.sp, "sp_load");
                Value *result = ctx_.builder.CreateSub(sp_val, 
                                                     ConstantInt::get(ctx_.SPType, n.count_));
                ctx_.builder.CreateStore(result, ctx_.sp);
            }
            
            void codegen(const ast::MoveRight &n) const {
                // sp += n.count_
                Value *sp_val = ctx_.builder.CreateLoad(ctx_.SPType, ctx_.sp, "sp_load");
                Value *result = ctx_.builder.CreateAdd(sp_val, 
                                                     ConstantInt::get(ctx_.SPType, n.count_));
                ctx_.builder.CreateStore(result, ctx_.sp);
            }
            
            void codegen(const ast::Add &n) const {
                // storage[sp] += n.count_
                Value *current_ptr = ctx_.current();
                Value *current_val = ctx_.builder.CreateLoad(ctx_.CellType, current_ptr, "current_load");
                Value *result = ctx_.builder.CreateAdd(current_val, 
                                                     ConstantInt::get(ctx_.CellType, n.count_));
                ctx_.builder.CreateStore(result, current_ptr);
            }
            
            void codegen(const ast::Minus &n) const {
                // storage[sp] -= n.count_
                Value *current_ptr = ctx_.current();
                Value *current_val = ctx_.builder.CreateLoad(ctx_.CellType, current_ptr, "current_load");
                Value *result = ctx_.builder.CreateSub(current_val, 
                                                     ConstantInt::get(ctx_.CellType, n.count_));
                ctx_.builder.CreateStore(result, current_ptr);
            }
            
            void codegen(const ast::Input &n) const {
                // storage[sp] = get_char()
                Value *current_ptr = ctx_.current();
                Value *input_char = ctx_.get_char();
                ctx_.builder.CreateStore(input_char, current_ptr);
            }
            
            void codegen(const ast::Output &n) const {
                // put_char(storage[sp])
                Value *current_ptr = ctx_.current();
                Value *current_val = ctx_.builder.CreateLoad(ctx_.CellType, current_ptr, "current_load");
                ctx_.put_char(current_val);
            }
            
            void codegen(const ast::Primitive &n) const {
                boost::apply_visitor(*this, n);
            }
            
            void codegen(const ast::Loop &n) const {
                // while(storage[sp] != 0) { ... }
                Function *TheFunction = ctx_.builder.GetInsertBlock()->getParent();
                BasicBlock *WhileBB = BasicBlock::Create(ctx_.ctx, "while_cond", TheFunction);
                BasicBlock *WBeginBB = BasicBlock::Create(ctx_.ctx, "while_body", TheFunction);
                BasicBlock *WEndBB = BasicBlock::Create(ctx_.ctx, "while_end", TheFunction);
                
                // Enter the while loop
                ctx_.builder.CreateBr(WhileBB);
                ctx_.builder.SetInsertPoint(WhileBB);
                
                // While condition
                Value *current_ptr = ctx_.current();
                Value *cur_val = ctx_.builder.CreateLoad(ctx_.CellType, current_ptr, "current_load");
                Value *zero = ConstantInt::get(ctx_.CellType, 0);
                Value *cond = ctx_.builder.CreateICmpNE(cur_val, zero);
                ctx_.builder.CreateCondBr(cond, WBeginBB, WEndBB);
                
                // While body
                ctx_.builder.SetInsertPoint(WBeginBB);
                for (const auto &cmd : *(n.commands_)) {
                    codegen(cmd);
                }
                
                // Jump back to condition
                ctx_.builder.CreateBr(WhileBB);

                // After while
                ctx_.builder.SetInsertPoint(WEndBB);
            }
            
            void codegen(const ast::Command &n) const {
                boost::apply_visitor(*this, n);
            }
            
            void codegen(const ast::Commands &n) const {
                for (const auto &cmd : n) {
                    codegen(cmd);
                }
            }
            
            void operator()(const ast::Program &n) const {
                codegen(n);
            }
            
            context &ctx_;
        };  // End of codegen_visitor
    }   // End of namespace details
        
    void codegen(Module &m, const ast::Program &n, unsigned int cell_size, size_t storage_size) {
        details::context ctx(m, cell_size, storage_size);
        details::codegen_visitor generator(ctx);
        generator(n);
    }
}   // End of namespace brainfuck
