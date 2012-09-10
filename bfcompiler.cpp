//
//  bfcompiler.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-9.
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

#include <string>
#include <iostream>
#include <fstream>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
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
        bool ret=parser::parse(s.begin(), s.end(), prog);
        if (!ret) {
            std::cerr << "Syntax error\n";
            exit(1);
        }
        
        llvm::LLVMContext &c=llvm::getGlobalContext();
        llvm::Module *m=new llvm::Module("brainfuck", c);
        brainfuck::codegen(*m, prog);
        
        llvm::raw_os_ostream os(ir);
        os << *m;
    }
}   // End of namespace brainfuck
