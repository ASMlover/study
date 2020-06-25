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


CODE(CONSTANT, 1)       // load the constant at index [arg]
CODE(NIL, 1)            // push `nil` onto the stack
CODE(FALSE, 1)          // push `false` onto the stack
CODE(TRUE, 1)           // push `true` onto the stack

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

// NOTE: the compiler assumes the following `STORE_*` instructions always
// immediately follow their corresponding `LOAD_*` ones

CODE(LOAD_LOCAL, 1)     // pushes the value in local slot [arg]
CODE(STORE_LOCAL, 0)    // stores the top of stack in local slot [arg], does not pop it
CODE(LOAD_UPVALUE, 1)   // pushes the value in upvalue [arg]
CODE(STORE_UPVALUE, 0)  // stores the top of stack in upvalue slot [arg], does not pop it

// pushes the value of the top-level variable in slot [arg]
CODE(LOAD_MODULE_VAR, 1)

// stores the top of stack in top-level variable slot [arg], does not pop it
CODE(STORE_MODULE_VAR, 0)

// pushes the value of the field in slot [arg] of the receiver of the current
// function, this is used for regular field accesses on "this" directly in
// methods, this instruction is faster than the more general `LOAD_FIELD`
// instruction
CODE(LOAD_FIELD_THIS, 1)

// stores the top of the stack in field slot [arg] in the receiver of the
// current value, does not pop the value, this instruction is faster than
// the more general `STORE_FIELD` instruction
CODE(STORE_FIELD_THIS, 0)

// pops an instance and pushes the value of the field in slot [arg] of it
CODE(LOAD_FIELD, 0)

// pops an instance and stores the subsequent top of stack in field slot
// [arg] in it, does not pop the value
CODE(STORE_FIELD, -1)

// pop and discard the top of stack
CODE(POP, -1)

// invoke the method with symbol [arg], the number indicates the number of
// arguments (not including the receiver)
CODE(CALL_0, 0)
CODE(CALL_1, -1)
CODE(CALL_2, -2)
CODE(CALL_3, -3)
CODE(CALL_4, -4)
CODE(CALL_5, -5)
CODE(CALL_6, -6)
CODE(CALL_7, -7)
CODE(CALL_8, -8)
CODE(CALL_9, -9)
CODE(CALL_10, -10)
CODE(CALL_11, -11)
CODE(CALL_12, -12)
CODE(CALL_13, -13)
CODE(CALL_14, -14)
CODE(CALL_15, -15)
CODE(CALL_16, -16)

// invoke a superclass method with symbol [arg], the number indicates the
// number of arguments (not including the receiver)
CODE(SUPER_0, 0)
CODE(SUPER_1, -1)
CODE(SUPER_2, -2)
CODE(SUPER_3, -3)
CODE(SUPER_4, -4)
CODE(SUPER_5, -5)
CODE(SUPER_6, -6)
CODE(SUPER_7, -7)
CODE(SUPER_8, -8)
CODE(SUPER_9, -9)
CODE(SUPER_10, -10)
CODE(SUPER_11, -11)
CODE(SUPER_12, -12)
CODE(SUPER_13, -13)
CODE(SUPER_14, -14)
CODE(SUPER_15, -15)
CODE(SUPER_16, -16)

// jump the instruction pointer [arg] forward
CODE(JUMP, 0)

// jump the instruction pointer [arg] backward
CODE(LOOP, 0)

// pop and if not truthy then the instruction pointer [arg] forward
CODE(JUMP_IF, -1)

// if the top of the stack is false, jump [arg] forward, otherwise pop
// and continue
CODE(AND, -1)

// if the top of the stack is non-false, jump [arg] forward, otherwise
// pop and continue
CODE(OR, -1)

// close the upvalue for the local on the top of the stack, then pop it
CODE(CLOSE_UPVALUE, -1)

// exit from the current function and return the value on the top of the
// stack
CODE(RETURN, 0)

// creates a closure for the function stored at [arg] in the constant table
//
// following the function argument is a number of arguments, two for each
// upvalue, the first is true if the variable being captured is a local (as
// opposed to an upvalue), and true second is the index of the local or
// upvalue being captured
//
// pushes the created closure
CODE(CLOSURE, 1)

// creates a new instance of a class
//
// assumes the class object is in slot zero, and replaces it with the new
// uninitialized instance of that class, this opcode is only emitted by the
// compiler-generated constructor metaclass methods
CODE(CONSTRUCT, 0)

// creates a new instance of a foreign class
//
// assumes the class object is in slot zero, and replaces it with the new
// uninitialized instance of that class, this opcode is only emitted by the
// compiler-generated constructor metaclass methods
CODE(FOREIGN_CONSTRUCT, 0)

// creates a class, top of stack is the superclass, below that is a string
// for the name of the class, byte [arg] is the number of fields in the class
CODE(CLASS, -1)

// creates a foreign class, top of stack is the superclass, below that is a
// string for the name of the class
CODE(FOREIGN_CLASS, -1)

// define a method for symbol [arg], the class receiving the mthod is
// popped off the stack, then the function defining the body is popped
//
// if a foreign method is being defined, the "function" will be a string
// identifying the foreign method, otherwise it will be a function or
// closure
CODE(METHOD_INSTANCE, -2)

// define a method for symbol [arg], the class whose metaclass will
// receive the method is popped off the stack, then the function defining
// the body is popped
//
// if a foreign method is being defined, the "function" will be a string
// identifying the foreign method, otherwise it will be a function or
// closure
CODE(METHOD_STATIC, -2)

// this is executed at the end of the module's body, pushes `nil` onto
// the stack as "return value" of the import statement and stroes the
// module as the most recently imported one
CODE(END_MODULE, 1)

// import a module whose name is the string stored at [arg] in the constant
// table
//
// pushes `nil` onto the stack so that the fiber for the imported module
// can replace that which a dummy value when it returns (fibers always
// return a value when resuming a caller)
CODE(IMPORT_MODULE, 1)

// import a variable from the most recently imported module, the name of
// the variable to import is at [arg] in the constant table, pushes the
// loaded variable's value
CODE(IMPORT_VARIABLE, 1)

// this pseudo-instruction indicates the end of the bytecode, it should
// always be preceded by `RETURN`, so is never actually executed
CODE(END, 0)

#undef CODE
#undef CODEF
