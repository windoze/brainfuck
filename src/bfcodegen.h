//
//  bfcodegen.h
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

// Upgrade to LLVM 18
// by 星灿长风v(StarWindv) on 2025/11/29


#include <llvm/IR/Module.h>
#include "bfast.h"

#ifndef brainfuck_bfcodegen_h
#define brainfuck_bfcodegen_h

namespace brainfuck {
    void codegen(llvm::Module &m, const ast::Program &n, unsigned int cell_size=8, size_t storage_size=30000);
}   // End of namespace brainfuck

#endif
