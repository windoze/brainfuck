//
//  main.cpp
//  brainfuck
//
//  Created by Xu Chen on 12-9-9.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include <string>
#include <iostream>
#include <fstream>
#include "bfcompiler.h"

// TODO: Use some real command line option parser
int main(int argc, const char * argv[])
{
    brainfuck::compiler comp;
    if (argc>1) {
        std::ifstream src(argv[1]);
        if (argc>2) {
            std::ofstream dest(argv[2]);
            comp.bfc(src, dest);
        } else {
            comp.bfc(src, std::cout);
        }
    } else {
        comp.bfc(std::cin, std::cout);
    }
    return 0;
}
