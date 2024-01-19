// Copyright (c) 2024 ASMlover. All rights reserved.
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
#pragma once

#include <ucontext.h>
#include <vector>
#include "common.hh"

namespace coro {

static constexpr int kStackSize = 1024 * 1024;
static constexpr int kCoroutineNum = 16;

enum class CoroStatus {
	CORO_STATUS_DEAD = 0,
	CORO_STATUS_READY,
	CORO_STATUS_RUNNING,
	CORO_STATUS_SUSPEND,
};

class Coroutine;
class Schedule;

typedef void (*pCoroFunction)(Schedule& s, void* args);

class Coroutine final : private UnCopyable {
	ucontext_t ucontext_{};
	CoroStatus status_{CoroStatus::CORO_STATUS_READY};
	char* stack_{};
	int cap_{};
	int size_{};

	pCoroFunction coro_start_;
	void* coro_args_;
public:
	Coroutine(Schedule& s, pCoroFunction co_func, void* co_args) noexcept;
	~Coroutine() noexcept;

	void acquire() noexcept;
	void release() noexcept;

	void start(Schedule& s) noexcept;

	void set_stack(const char* stack) noexcept;

	inline ucontext_t& ucontext() noexcept { return ucontext_; }
	inline CoroStatus status() const noexcept { return status_; }
	inline void set_status(CoroStatus status) noexcept { status_ = status; }
	inline const char* stack() const noexcept { return stack_; }
	inline int cap() const noexcept { return cap_; }
	inline void set_cap(int cap) noexcept { cap_ = cap; }
	inline int size() const noexcept { return size_; }
	inline void set_size(int size) noexcept { size_ = size; }
};

class Schedule final : private UnCopyable {
	char* stack_{};
	ucontext_t main_{};
	int coro_num_{};
	int coro_cap_{kCoroutineNum};
	int running_id_{-1};
	std::vector<Coroutine*> coro_list_;
public:
	Schedule() noexcept;
	~Schedule() noexcept;

	int coroutine_create(pCoroFunction coro_func, void* coro_args) noexcept;
	void coroutine_resume(int coro_id) noexcept;
	void coroutine_yield() noexcept;

	CoroStatus get_coroutine_status(int coro_id) const noexcept;
	int get_running_coroutine_id() const noexcept;

	void save_stack(Coroutine* coro, const char* top) noexcept;
	void start_running_coroutine() noexcept;

	inline Coroutine* get_coroutine(int coro_id) noexcept { return coro_list_[coro_id]; }
	inline Coroutine* get_running_coroutine() noexcept { return coro_list_[running_id_]; }
};

}
