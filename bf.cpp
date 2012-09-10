//
//  bf.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-9.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include <string>
#include <iostream>
#include <fstream>
#include "bfjit.h"

// TODO: Use some real command line option parser
int main(int argc, const char * argv[])
{
    brainfuck::jit::main_func_type fp;
    if (argc>1) {
        std::ifstream src(argv[1]);
        fp=brainfuck::jit::compile(src);
    } else {
        fp=brainfuck::jit::compile(std::cin);
    }
    fp();
    return 0;
}
