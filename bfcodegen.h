//
//  bfcodegen.h
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

#include <llvm/Module.h>
#include "bfast.h"

#ifndef brainfuck_bfcodegen_h
#define brainfuck_bfcodegen_h

namespace brainfuck {
    void codegen(llvm::Module &m, const ast::Program &n, unsigned int cell_size=8, size_t storage_size=30000);
}   // End of namespace brainfuck

#endif
