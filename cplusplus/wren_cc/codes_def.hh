// Copyright (c) 2019 ASMlover. All rights reserved.
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

#ifndef CODEF
# define CODEF(c)
#endif

#ifndef CODE
# define CODE(c) CODEF(c)
#endif

CODE(CONSTANT)     // load the constant at index [arg]
CODE(NIL)          // push `nil` into the stack
CODE(FALSE)        // push `false` into the stack
CODE(TRUE)         // push `true` into the stack

// pushes the value in the given local slot
CODE(LOAD_LOCAL_0)
CODE(LOAD_LOCAL_1)
CODE(LOAD_LOCAL_2)
CODE(LOAD_LOCAL_3)
CODE(LOAD_LOCAL_4)
CODE(LOAD_LOCAL_5)
CODE(LOAD_LOCAL_6)
CODE(LOAD_LOCAL_7)
CODE(LOAD_LOCAL_8)

CODE(LOAD_LOCAL)   // push the value in local slot [arg]
CODE(STORE_LOCAL)  // store the top of the stack in local slot [arg], not pop it
CODE(LOAD_UPVALUE) // push the value in upvalue [arg]
CODE(STORE_UPVALUE)// store the top of stack in upvalue [arg], does not pop it

// pushes the value of the top-level variable in slot [arg]
CODE(LOAD_MODULE_VAR)

// stores the top of stack in the top-level variable slot [arg], not pop it
CODE(STORE_MODULE_VAR)

// push the value of the field in slot [arg] of the receiver of the current
// function. this is used for regular field access on `this` directly in
// methods. this instruction is faster than the more general LOAD_FIELD
// instruction
CODE(LOAD_FIELD_THIS)
// stroe the top of the stack in field slot [arg] in the receiver of the current
// value, does not pop the value, this instruction is faster than the more
// general LOAD_FIELD instruction
CODE(STORE_FIELD_THIS)

// pop an instance and push the value of the field in slot [arg] of it
CODE(LOAD_FIELD)
// pop an instance and store the subsequent top of stack in field slot [arg]
// in it. does not pop the value
CODE(STORE_FIELD)
CODE(POP)          // pop and discard the top of stack
CODE(DUP)          // push a copy of the value currently on the top of stack

// invoke the method with symbol [arg], the number indicates the number of
// arguments (not including the receiver)
CODE(CALL_0)
CODE(CALL_1)
CODE(CALL_2)
CODE(CALL_3)
CODE(CALL_4)
CODE(CALL_5)
CODE(CALL_6)
CODE(CALL_7)
CODE(CALL_8)
CODE(CALL_9)
CODE(CALL_10)
CODE(CALL_11)
CODE(CALL_12)
CODE(CALL_13)
CODE(CALL_14)
CODE(CALL_15)
CODE(CALL_16)

// invoke a superclass method with symbol [arg], the number indicates the
// number of arguments (not including the receiver)
CODE(SUPER_0)
CODE(SUPER_1)
CODE(SUPER_2)
CODE(SUPER_3)
CODE(SUPER_4)
CODE(SUPER_5)
CODE(SUPER_6)
CODE(SUPER_7)
CODE(SUPER_8)
CODE(SUPER_9)
CODE(SUPER_10)
CODE(SUPER_11)
CODE(SUPER_12)
CODE(SUPER_13)
CODE(SUPER_14)
CODE(SUPER_15)
CODE(SUPER_16)

CODE(JUMP)         // jump the instruction pointer [arg] forward

// jump the instruction pointer [arg] backward. pop and discard the
// top of the stack
CODE(LOOP)

CODE(JUMP_IF)      // pop and if not truthy then jump the instruction pointer [arg] forward

CODE(AND)          // if the top of the stack is false jump [arg], or pop and continue
CODE(OR)           // if the top of the stack if non-false jump [arg], or pop and continue
CODE(IS)           // pop [a] then [b] and push true if [b] is an instance of [a]

// close the upvalue for the local on the top of the stack, then pop it.
CODE(CLOSE_UPVALUE)
CODE(RETURN)       // exit from the current function and return the value on the top of stack

// creates a closure for the function stored at [arg] in the constant table
//
// following the function argument is a number of arguments, two for each
// upvalue. the first is non-zero if the variable being captured is a local
// and the second is the index of the local or upvalue being captured.
//
// pushes the created closure object
CODE(CLOSURE)

// creates a class, top of stack is the superclass, or `nil` if the class
// inherits Object, below that is a string for the name of the class, byte
// [arg] is the number of fields in the class
CODE(CLASS)

// define a method for symbol [arg] the class receiving the method is popped
// off the stack, then the function defining the body is popped
CODE(METHOD_INSTANCE)

// define a method for symbol [arg] the class whose metaclass will receive
// the method is popped off the stack, then the function defining the body
// is popped
CODE(METHOD_STATIC)

// load the module whose name is stored in string constant [arg], pushes
// nullptr onto the stack, if the module has already been loaded, does nothing
// else, otherwise it creates a fiber to run the desired module and switches
// to that, when that fiber is done the current one is resumed
CODE(LOAD_MODULE)

CODE(END)

#undef CODE
#undef CODEF
