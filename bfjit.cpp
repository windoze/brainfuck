//
//  bfjit.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#endif

#include <stdio.h>
#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Value.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>

#include "bfast.h"
#include "bfparser.h"
#include "bfcodegen.h"
#include "bfjit.h"

using namespace llvm;

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
        }
        
        main_func_type jit_engine::compile(const ast::Program &prog) {
            llvm::LLVMContext &c=llvm::getGlobalContext();
            llvm::Module *m=new llvm::Module("brainfuck", c);
            brainfuck::codegen(*m, prog);
            //m->dump();
            
            std::string ErrStr;
            ExecutionEngine *TheExecutionEngine = EngineBuilder(m).setErrorStr(&ErrStr).create();
            if (!TheExecutionEngine) {
                fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
                exit(1);
            }
            Function *f_main=m->getFunction("main");
            
            // Optimize, target dependant
            if(optimization_level_>0)
            {
                FunctionPassManager OurFPM(m);
                
                // Set up the optimizer pipeline.  Start with registering info about how the
                // target lays out data structures.
                OurFPM.add(new TargetData(*TheExecutionEngine->getTargetData()));
                // Provide basic AliasAnalysis support for GVN.
                OurFPM.add(createBasicAliasAnalysisPass());
                // Do simple "peephole" optimizations and bit-twiddling optzns.
                OurFPM.add(createInstructionCombiningPass());
                // Reassociate expressions.
                OurFPM.add(createReassociatePass());
                // Eliminate Common SubExpressions.
                OurFPM.add(createGVNPass());
                // Simplify the control flow graph (deleting unreachable blocks, etc).
                OurFPM.add(createCFGSimplificationPass());
                //OurFPM.add(createLoopSimplifyPass());
                //OurFPM.add(createBlockPlacementPass());
                //OurFPM.add(createConstantPropagationPass());
                
                OurFPM.doInitialization();
             
                OurFPM.run(*f_main);
            }
            
            //m->dump();
            
            void *rp=TheExecutionEngine->getPointerToFunction(f_main);
            main_func_type fp=reinterpret_cast<main_func_type>(rp);
            return fp;
        }
        
        main_func_type compile(std::istream &src) {
            jit_engine engine;
            ast::Program prog;
            std::string s((std::istreambuf_iterator<char>(src)),
                          std::istreambuf_iterator<char>());
            bool ret=parser::parse(s.begin(), s.end(), prog);
            if (!ret) {
                std::cerr << "Syntax error\n";
                exit(1);
            }
            return engine.compile(prog);
        }
    }   // End of namespace jit
}   // End of namespace brainfuck

