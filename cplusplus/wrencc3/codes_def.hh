// Copyright (c) 2020 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// this defines the bytecode instructions used by the VM, it does so by
// invoking an CODE() macro which is expected to be defined at the point
// that this is included (see: http://en.wikipedia.org/wiki/X_Macro for more)
//
// the first argument is the name of the opcode, the second is its "stack
// effect" -- the amount that the opcode changes the size of the stack, a
// stack effect of 1 means it pushes a value and the stack grows on larger,
// -2 means it pops two values, etc.
//
// note that the order of instructions here affects the order of the dispatch
// table in the VM's interpreter loop. that in turn affects caching which
// affects overall performance, take care to run benchmarks if you change
// the order here

#ifndef CODEF
# define CODEF(c, n)
#endif

#ifndef CODE
# define CODE(c, n) CODEF(c, n)
#endif


CODE(CONSTANT, 1)     // load the constant at index [arg]
CODE(NIL, 1)          // push `nil` onto the stack
CODE(FALSE, 1)        // push `false` onto the stack
CODE(TRUE, 1)         // push `true` onto the stack

// pushes the value in the given local slot
CODE(LOAD_LOCAL_0, 1)
CODE(LOAD_LOCAL_1, 1)
CODE(LOAD_LOCAL_2, 1)
CODE(LOAD_LOCAL_3, 1)
CODE(LOAD_LOCAL_4, 1)
CODE(LOAD_LOCAL_5, 1)
CODE(LOAD_LOCAL_6, 1)
CODE(LOAD_LOCAL_7, 1)
CODE(LOAD_LOCAL_8, 1)

// NOTE: the compiler assumes the following *_STORE instructions always
// immediately follow their corresponding *_LOAD ones

CODE(LOAD_LOCAL, 1)   // pushes the value in local slot [arg]

#undef CODE
#undef CODEF
