//
//  bfcodegen.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include "bfcodegen.h"
#include <llvm/LLVMContext.h>
#include <llvm/Value.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>

using namespace llvm;
namespace brainfuck {
    namespace details {
        
        inline ConstantInt *const_int(IntegerType *t, int64_t n=0, bool is_signed=false) {
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
            , storage(new GlobalVariable(m,
                                         StorageType,
                                         false,
                                         GlobalValue::InternalLinkage,
                                         0,
                                         "s"))
            , sp(new GlobalVariable(m,
                                    SPType,
                                    false,
                                    GlobalValue::InternalLinkage,
                                    0,
                                    "sp"))
            , get_char_(0)
            , put_char_(0)
            , entry_(0)
            {
                // Initialize storage
                {
                    Constant *init=ConstantArray::get(StorageType, std::vector<Constant *>(0, const_int(IntegerType::getInt32Ty(ctx))));
                    storage->setInitializer(init);
                }
                // Initialize sp
                {
                    sp->setInitializer(const_int(SPType, 0));
                }
                // Initialize external functions
                {
                    FunctionType *FT = FunctionType::get(CellType, false);
                    get_char_=Function::Create(FT, Function::ExternalLinkage, "getchar", &module);
                }
                {
                    std::vector<Type *> args(1, CellType);
                    FunctionType *FT = FunctionType::get(Type::getVoidTy(ctx), args, false);
                    put_char_=Function::Create(FT, Function::ExternalLinkage, "putchar", &module);
                }
                // Initialize entry point, which is a function with name 'main'
                {
                    FunctionType *MainType=FunctionType::get(Type::getVoidTy(m.getContext()), false);
                    entry_ = Function::Create(MainType, Function::ExternalLinkage, "main", &m);
                    BasicBlock *BB = BasicBlock::Create(m.getContext(), "", entry_);
                    builder.SetInsertPoint(BB);
                }
            }
            
            ~context() {
                // Close up function 'main'
                builder.CreateRetVoid();
                llvm::verifyFunction(*entry_);
            }
            
            /// Return pointer to current
            Value *current() {
                std::vector<Value *> idx;
                idx.push_back(const_int(IntegerType::getInt32Ty(ctx)));
                idx.push_back(builder.CreateLoad(sp));
                return builder.Insert(GetElementPtrInst::Create(storage, idx));
            }
            
            Instruction *get_char() {
                return builder.CreateCall(get_char_);
            }

            Instruction *put_char(Value *arg) {
                return builder.CreateCall(put_char_, arg);
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
                // sp-=n.count_
                Value *result=ctx_.builder.CreateSub(ctx_.builder.CreateLoad(ctx_.sp),
                                                     const_int(ctx_.SPType, n.count_));
                ctx_.builder.CreateStore(result, ctx_.sp);
            }
            
            void codegen(const ast::MoveRight &n) const {
                // sp+=n.count_
                Value *result=ctx_.builder.CreateAdd(ctx_.builder.CreateLoad(ctx_.sp),
                                                     const_int(ctx_.SPType, n.count_));
                ctx_.builder.CreateStore(result, ctx_.sp);
            }
            
            void codegen(const ast::Add &n) const {
                // storage[sp]+=n.count_
                Value *current=ctx_.current();
                Value *result=ctx_.builder.CreateAdd(ctx_.builder.CreateLoad(current),
                                                     const_int(ctx_.CellType, n.count_));
                ctx_.builder.CreateStore(result, current);
            }
            
            void codegen(const ast::Minus &n) const {
                // storage[sp]-=n.count_
                Value *current=ctx_.current();
                Value *result=ctx_.builder.CreateSub(ctx_.builder.CreateLoad(current),
                                                     const_int(ctx_.CellType, n.count_));
                ctx_.builder.CreateStore(result, current);
            }
            
            void codegen(const ast::Input &n) const {
                // storage[sp]=get_char()
                ctx_.builder.CreateStore(ctx_.get_char(), ctx_.current());
            }
            
            void codegen(const ast::Output &n) const {
                // put_char(storage[sp])
                ctx_.put_char(ctx_.builder.CreateLoad(ctx_.current()));
            }
            
            void codegen(const ast::Primitive &n) const {
                boost::apply_visitor(*this, n);
            }
            
            void codegen(const ast::Loop &n) const {
                // while(storage[sp]) { ... }
                //
                // while:
                //      test storage[sp]!=0
                //      branch true :wbegin, false :wend
                // wbegin:
                //      body
                //      goto :while
                // wend:
                //      after loop
                Function *TheFunction = ctx_.builder.GetInsertBlock()->getParent();
                BasicBlock *WhileBB = BasicBlock::Create(ctx_.ctx, "", TheFunction);
                BasicBlock *WBeginBB = BasicBlock::Create(ctx_.ctx, "", TheFunction);
                BasicBlock *WEndBB = BasicBlock::Create(ctx_.ctx, "", TheFunction);
                
                // Enter the while loop
                ctx_.builder.CreateBr(WhileBB);
                ctx_.builder.SetInsertPoint(WhileBB);
                
                // While condition
                Value *cur=ctx_.builder.CreateLoad(ctx_.current());
                Value *cond=ctx_.builder.CreateICmpNE(cur, const_int(ctx_.CellType, 0));
                ctx_.builder.CreateCondBr(cond, WBeginBB, WEndBB);
                
                // While body
                ctx_.builder.SetInsertPoint(WBeginBB);
                codegen(*(n.commands_));
                
                // Jump to begin
                ctx_.builder.CreateBr(WhileBB);

                // After while
                ctx_.builder.SetInsertPoint(WEndBB);
            }
            
            void codegen(const ast::Command &n) const {
                boost::apply_visitor(*this, n);
            }
            
            void codegen(const ast::Commands &n) const {
                for (ast::Commands::const_iterator i=n.begin(); i!=n.end(); ++i) {
                    codegen(*i);
                }
            }
            
            context &ctx_;
        };  // End of codegen_visitor
    }   // End of namespace details
        
    void codegen(Module &m, const ast::Program &n, unsigned int cell_size, size_t storage_size) {
        {
            details::context ctx(m, cell_size, storage_size);
            details::codegen_visitor generator(ctx);
            generator(n);
        }
    }
}   // End of namespace brainfuck
