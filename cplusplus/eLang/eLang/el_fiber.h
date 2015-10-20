// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_FIBER_HEADER_H__
#define __EL_FIBER_HEADER_H__

#include "el_block.h"
#include "el_object.h"
#include "el_upvalue.h"

namespace el {

class Environment;
class Expr;
class Interpreter;

class Fiber : private UnCopyable {
  struct CallFrame {
    int   ip;
    int   stack_start;
    Value receiver;
    Value block;

    CallFrame(void)
      : ip(0)
      , stack_start(0)
      , receiver()
      , block() {
    }

    CallFrame(int start, const Value& r, const Value& b)
      : ip(0)
      , stack_start(start)
      , receiver(r)
      , block(b) {
    }

    inline const BlockObject& Block(void) const {
      return *(block.AsBlock());
    }
  };

  bool             is_running_;
  Interpreter&     interpreter_;
  Array<Value>     stack_;
  Stack<CallFrame> call_frames_;
  Ref<Upvalue>     open_upvalues_;
public:
  Fiber(Interpreter& interpreter, const Value& block);

  inline bool IsRunning(void) const {
    return is_running_ && !IsDone();
  }

  bool IsDone(void) const;
  Value Execute(void);

  inline Interpreter& GetInterpreter(void) const {
    return interpreter_;
  }

  inline void Pause(void) {
    is_running_ = false;
  }

  const Value& Nil(void);
  const Value& CreatBoolean(bool value);
  Value CreateNumber(double value);
  Value CreateString(const String& value);

  void CallBlock(const Value& receiver,
      const Value& block_obj, const ArgReader& args);
  void Error(const String& message);
  int GetCallstackDepth(void) const;
private:
  Value Load(const CallFrame& frame, int reg);
  void Store(const CallFrame& frame, int reg, const Value& value);
  void PopCallFrame(void);
  void StoreMessageResult(const Value& result);
  Value SendMessage(StringId message_id, int receiver_reg, int num_args);
  const Value& Self(void);
  Ref<Upvalue> CaptureUpvalue(int stack_index);
#ifdef EL_TRACE_INSTRUCTIONS
  void TraceInstruction(Instruction instruction);
  void TraceStack(void);
#endif
};

}

#endif  // __EL_FIBER_HEADER_H__
