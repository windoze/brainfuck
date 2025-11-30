//
//  bfjit.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

// Upgrade to LLVM 18
// by 星灿长风v(StarWindv) on 2025/11/29


#include <stdio.h>
#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

#include "bfast.h"
#include "bfparser.h"
#include "bfcodegen.h"
#include "bfjit.h"

using namespace llvm;
using namespace llvm::orc;

namespace brainfuck {
    namespace jit {
        struct jit_engine {
            jit_engine(int optimization_level=0);
            main_func_type compile(const ast::Program &prog);
            
            int optimization_level_;
        };  // End of jit_engine

        jit_engine::jit_engine(int optimization_level)
        : optimization_level_(optimization_level)
        {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();
        }
        
        main_func_type jit_engine::compile(const ast::Program &prog) {
            auto context = std::make_unique<LLVMContext>();
            auto module = std::make_unique<Module>("brainfuck", *context);
            brainfuck::codegen(*module, prog);
            
            // Apply optimizations
            if (optimization_level_ > 0) {
                LoopAnalysisManager LAM;
                FunctionAnalysisManager FAM;
                CGSCCAnalysisManager CGAM;
                ModuleAnalysisManager MAM;
                
                PassBuilder PB;
                
                PB.registerModuleAnalyses(MAM);
                PB.registerCGSCCAnalyses(CGAM);
                PB.registerFunctionAnalyses(FAM);
                PB.registerLoopAnalyses(LAM);
                PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
                
                ModulePassManager MPM;
                if (optimization_level_ == 1) {
              //      MPM = PB.buildO1DefaultPipeline(OptimizationLevel::O1);
		      MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O1);
                } else if (optimization_level_ >= 2) {
              //      MPM = PB.buildO2DefaultPipeline(OptimizationLevel::O2);
		      MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
                }
                
                MPM.run(*module, MAM);
            }
            
            // Create JIT
            auto JIT = LLJITBuilder().create();
            if (!JIT) {
                fprintf(stderr, "Could not create LLJIT: %s\n", 
                        toString(JIT.takeError()).c_str());
                exit(1);
            }
            
            // Add module to JIT
            ThreadSafeModule TSM(std::move(module), std::move(context));
            if (auto Err = (*JIT)->addIRModule(std::move(TSM))) {
                fprintf(stderr, "Could not add IR module: %s\n", 
                        toString(std::move(Err)).c_str());
                exit(1);
            }
            
            // Look up main function
            auto MainSym = (*JIT)->lookup("main");
            if (!MainSym) {
                fprintf(stderr, "Could not find main function: %s\n", 
                        toString(MainSym.takeError()).c_str());
                exit(1);
            }
            
           //  void *fp = MainSym->getAddress();
                void *fp = reinterpret_cast<void*>(MainSym->getValue());
            return reinterpret_cast<main_func_type>(fp);
        }
        
        main_func_type compile(std::istream &src) {
            jit_engine engine;
            ast::Program prog;
            std::string s((std::istreambuf_iterator<char>(src)),
                          std::istreambuf_iterator<char>());
            bool ret = parser::parse(s.begin(), s.end(), prog);
            if (!ret) {
                std::cerr << "Syntax error\n";
                exit(1);
            }
            return engine.compile(prog);
        }
    }   // End of namespace jit
}   // End of namespace brainfuck
