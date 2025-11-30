//
//  bfcompiler.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-9.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

// Upgrade to LLVM 18
// by 星灿长风v(StarWindv) on 2025/11/29


#include <string>
#include <iostream>
#include <fstream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>
#include "bfparser.h"
#include "bfast.h"
#include "bfcodegen.h"
#include "bfcompiler.h"

namespace brainfuck {
    void compiler::bfc(std::istream &src, std::ostream &ir) {
        std::string s((std::istreambuf_iterator<char>(src)),
                      std::istreambuf_iterator<char>());
        ast::Program prog;
        bool ret = parser::parse(s.begin(), s.end(), prog);
        if (!ret) {
            std::cerr << "Syntax error\n";
            exit(1);
        }
        
        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("brainfuck", context);
        brainfuck::codegen(*module, prog);
        
        llvm::raw_os_ostream os(ir);
        module->print(os, nullptr);
    }
}   // End of namespace brainfuck
