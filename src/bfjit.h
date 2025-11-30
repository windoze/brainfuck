//
//  bfjit.h
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include <istream>

#ifndef brainfuck_bfjit_h
#define brainfuck_bfjit_h

namespace brainfuck {
    namespace jit {
        typedef void (*main_func_type)();
        
        main_func_type compile(std::istream &is);
    }   // End of namespace jit
}   // End of namespace brainfuck

#endif
