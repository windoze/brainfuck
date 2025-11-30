//
//  bfcompiler.h
//  brainfuck
//
//  Created by Xu Chen on 12-9-9.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include <string>
#include <istream>
#include <ostream>

#ifndef brainfuck_bfcompiler_h
#define brainfuck_bfcompiler_h

namespace brainfuck {
    struct compiler {
        // Compile source into IR
        void bfc(std::istream &src, std::ostream &ir);
    };
}   // End of namespace brainfuck

#endif
