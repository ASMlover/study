/*
 * Copyright (c) 2024 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

static Value clockNative(int argCount, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");

  for (int i = vm.frameCount - 1; i >= 0; --i) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->closure->function;
    sz_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (NULL == function->name)
      fprintf(stderr, "script\n");
    else
      fprintf(stderr, "%s(...)\n", function->name->chars);
  }

  resetStack();
}

static void defineNative(const char* name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expect %d arguments but got %d.", closure->function->arity, argCount);
    return false;
  }

  if (vm.frameCount >= LOXC_FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_BOUND_METHOD:
    {
      ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
      vm.stackTop[-argCount - 1] = bound->receiver;
      return call(bound->method, argCount);
    } break;
    case OBJ_CLASS:
    {
      ObjClass* klass = AS_CLASS(callee);
      vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
      Value initializer;
      if (tableGet(&klass->methods, vm.initString, &initializer)) {
        return call(AS_CLOSURE(initializer), argCount);
      }
      else if (0 != argCount) {
        runtimeError("Expect 0 arguments but got %d.", argCount);
        return false;
      }
      return true;
    } break;
    case OBJ_CLOSURE: return call(AS_CLOSURE(callee), argCount);
    case OBJ_NATIVE:
    {
      NativeFn native = AS_NATIVE(callee);
      Value result = native(argCount, vm.stackTop - argCount);
      vm.stackTop -= argCount + 1;
      push(result);
      return true;
    } break;
    default: break;
    }
  }

  runtimeError("Can only call functions and classes.");
  return false;
}

static bool invokeFromClass(ObjClass* klass, ObjString* methodName, int argCount) {
  Value method;
  if (!tableGet(&klass->methods, methodName, &method)) {
    runtimeError("Undefined property `%s`.", methodName->chars);
    return false;
  }
  return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString* methodName, int argCount) {
  Value receiver = peek(argCount);
  if (!IS_INSTANCE(receiver)) {
    runtimeError("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = AS_INSTANCE(receiver);

  Value method;
  if (tableGet(&instance->fields, methodName, &method)) {
    vm.stackTop[-argCount - 1] = method;
    return callValue(method, argCount);
  }

  return invokeFromClass(instance->klass, methodName, argCount);
}

static bool bindMethod(ObjClass* klass, ObjString* methodName) {
  Value method;
  if (!tableGet(&klass->methods, methodName, &method)) {
    runtimeError("Undefined property `%s`.", methodName->chars);
    return false;
  }

  ObjBoundMethod* boundMethod = newBoundMethod(peek(0), AS_CLOSURE(method));

  pop();
  push(OBJ_VAL(boundMethod));
  return true;
}

static ObjUpvalue* captureUpvalue(Value* local) {
  ObjUpvalue* prevUpvalue = NULL;
  ObjUpvalue* upvalue = vm.openUpvalues;
  while (NULL != upvalue && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (NULL != upvalue && upvalue->location == local)
    return upvalue;

  ObjUpvalue* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (NULL != prevUpvalue)
    vm.openUpvalues = createdUpvalue;
  else
    prevUpvalue->next = createdUpvalue;
  return createdUpvalue;
}

static void closeUpvalues(Value* last) {
  while (NULL != vm.openUpvalues && vm.openUpvalues->location >= last) {
    ObjUpvalue* upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

static void defineMethod(ObjString* methodName) {
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));
  tableSet(&klass->methods, methodName, method);
  pop();
}

static void concatenate() {
  ObjString* b = AS_STRING(peek(0));
  ObjString* a = AS_STRING(peek(1));

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = 0;

  ObjString* result = takeString(chars, length);
  pop();
  pop();

  push(OBJ_VAL(result));
}

static InterpretResult run() {
  CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE()                         (*frame->ip++)
#define READ_SHORT()                        (frame->ip += 2, (u16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT()                     (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING()                       AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op)\
  do {\
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {\
      runtimeError("Operands must be numbers.");\
      return INTERPRET_RUNTIME_ERROR;\
    }\
    double b = AS_NUMBER(pop());\
    double a = AS_NUMBER(pop());\
    push(valueType(a op b));\
  } while (false)

  for (;;) {
#ifdef LOXC_DEBUG_TRACE_EXECUTION
    fprintf(stdout, "          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; ++slot) {
      fprintf(stdout, "[ ");
      printValue(*slot);
      fprintf(stdout, " ]");
    }
    fprintf(stdout, "\n");

    disassebleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
#endif

    u8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT:
      {
        Value constant = READ_CONSTANT();
        push(constant);
      } break;
    case OP_NIL: push(NIL_VAL); break;
    case OP_TRUE: push(BOOL_VAL(true)); break;
    case OP_FALSE: push(BOOL_VAL(false)); break;
    case OP_POP: pop(); break;
    case OP_GET_LOCAL:
      {
        u8_t slot = READ_BYTE();
        push(frame->slots[slot]);
      } break;
    case OP_SET_LOCAL:
      {
        u8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
      } break;
    case OP_GET_GLOBAL:
      {
        ObjString* name = READ_STRING();
        Value value;
        if (!tableGet(&vm.globals, name, &value)) {
          runtimeError("Undefined variable `%s`.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
      } break;
    case OP_DEFINE_GLOBAL:
      {
        ObjString* name = READ_STRING();
        tableSet(&vm.globals, name, peek(0));
        pop();
      } break;
    case OP_SET_GLOBAL:
      {
        ObjString* name = READ_STRING();
        if (tableSet(&vm.globals, name, peek(0))) {
          tableDelete(&vm.globals, name);
          runtimeError("Undefined variable `%s`.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
      } break;
    case OP_GET_UPVALUE:
      {
        u8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
      } break;
    case OP_SET_UPVALUE:
      {
        u8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
      } break;
    case OP_GET_PROPERTY:
      {
        if (!IS_INSTANCE(peek(0))) {
          runtimeError("Only instances have properties.");
          return INTERPRET_RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(0));
        ObjString* name = READ_STRING();

        Value value;
        if (tableGet(&instance->fields, name, &value)) {
          pop();
          push(value);
          break;
        }

        if (!bindMethod(instance->klass, name))
          return INTERPRET_RUNTIME_ERROR;
      } break;
    case OP_SET_PROPERTY:
      {
        if (!IS_INSTANCE(peek(1))) {
          runtimeError("Only instances have fields.");
          return INTERPRET_RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(1));
        tableSet(&instance->fields, READ_STRING(), peek(0));
        Value value = pop();
        pop();
        push(value);
      } break;
    case OP_GET_SUPER:
      {
        ObjString* name = READ_STRING();
        ObjClass* superclass = AS_CLASS(pop());

        if (!bindMethod(superclass, name))
          return INTERPRET_RUNTIME_ERROR;
      } break;
    case OP_EQUAL:
      {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
      } break;
    case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
    case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
    case OP_ADD:
      {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        }
        else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        }
        else {
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
      } break;
    case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
    case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
    case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
    case OP_NOT:      push(BOOL_VAL(isFalsey(pop()))); break;
    case OP_NEGATE:
      {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operands must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
      } break;
    case OP_PRINT:
      {
        printValue(pop());
        fprintf(stdout, "\n");
      } break;
    case OP_JUMP:
      {
        u16_t offset = READ_SHORT();
        frame->ip += offset;
      } break;
    case OP_JUMP_IF_FALSE:
      {
        u16_t offset = READ_SHORT();
        if (isFalsey(peek(0)))
          frame->ip += offset;
      } break;
    case OP_LOOP:
      {
        u16_t offset = READ_SHORT();
        frame->ip -= offset;
      } break;
    case OP_CALL:
      {
        int argCount = READ_BYTE();
        if (!callValue(peek(argCount), argCount))
          return INTERPRET_RUNTIME_ERROR;

        frame = &vm.frames[vm.frameCount - 1];
      } break;
    case OP_INVOKE:
      {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        if (!invoke(method, argCount))
          return INTERPRET_RUNTIME_ERROR;
        frame = &vm.frames[vm.frameCount - 1];
      } break;
    case OP_SUPER_INVOKE:
      {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();
        ObjClass* superclass = AS_CLASS(pop());
        if (!invokeFromClass(superclass, method, argCount))
          return INTERPRET_RUNTIME_ERROR;
        frame = &vm.frames[vm.frameCount - 1];
      } break;
    case OP_CLOSURE:
      {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = newClosure(function);
        push(OBJ_VAL(closure));

        for (int i = 0; i < closure->upvalueCount; ++i) {
          u8_t isLocal = READ_BYTE();
          u8_t index = READ_BYTE();
          if (isLocal)
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          else
            closure->upvalues[i] = frame->closure->upvalues[index];
        }
      } break;
    case OP_CLOSE_UPVALUE:
      closeUpvalues(vm.stackTop - 1);
      pop();
      break;
    }
  }

#undef BINARY_OP
#undef READ_STRING
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE

  return INTERPRET_OK;
}

void initVM() {
  resetStack();

  vm.objects = NULL;
  vm.bytesAllocated = 0;
  vm.nextGC = 1048576;  // 1024 * 1024;
  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.grayStack = NULL;

  initTable(&vm.globals);
  initTable(&vm.strings);

  vm.initString = copyString("init", 4);

  defineNative("clock", clockNative);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  vm.initString = NULL;

  freeObjects();
}

InterpretResult interpret(const char* sourceCode) { return INTERPRET_OK; }

void push(Value value) {
  *vm.stackTop++ = value;
}

Value pop() {
  return *(--vm.stackTop);
}
