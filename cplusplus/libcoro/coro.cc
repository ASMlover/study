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
#include <cstdint>
#include <cstring>
#include "coro.hh"

namespace coro {

Coroutine::Coroutine(Schedule& s, pCoroFunction co_func, void* co_args) noexcept
	: coro_start_{co_func}
	, coro_args_{co_args} {
}

Coroutine::~Coroutine() noexcept {
}

void Coroutine::acquire() noexcept {
	if (stack_ != nullptr)
		delete [] stack_;
	stack_ = new char[cap_];
}

void Coroutine::release() noexcept {
	if (stack_ != nullptr)
		delete [] stack_;
}

void Coroutine::start(Schedule& s) noexcept {
	if (coro_start_)
		coro_start_(s, coro_args_);
}

void Coroutine::set_stack(const char* stack) noexcept {
	::memcpy(stack_, stack, size_);
}

static void start_func(uint32_t low_addr, uint32_t high_addr) noexcept {
	uintptr_t sched_ptr = ((uintptr_t)high_addr << 32) + (uintptr_t)low_addr;
	Schedule* sched = (Schedule*)sched_ptr;

	sched->start_running_coroutine();
}

Schedule::Schedule() noexcept {
	stack_ = new char[kStackSize];
	coro_list_ = std::vector<Coroutine*>(kCoroutineNum, nullptr);
}

Schedule::~Schedule() noexcept {
	for (int i = 0; i < coro_cap_; ++i) {
		Coroutine* coro = coro_list_[i];
		if (coro != nullptr)
			coro->release();
	}
	delete [] stack_;
}

int Schedule::coroutine_create(pCoroFunction coro_func, void* coro_args) noexcept {
	Coroutine* coro = new Coroutine(*this, coro_func, coro_args);

	if (coro_num_ >= coro_cap_) {
		coro_list_.push_back(coro);
		++coro_num_;
		coro_cap_ = coro_list_.capacity();
		return coro_num_ - 1;
	}
	else {
		for (int i = 0; i < coro_cap_; ++i) {
			int coro_id = (i + coro_num_) % coro_cap_;
			if (coro_list_[coro_id] == nullptr) {
				coro_list_[coro_id] = coro;
				++coro_num_;
				return coro_id;
			}
		}
		return -1;
	}
}

void Schedule::coroutine_resume(int coro_id) noexcept {
	Coroutine* coro = coro_list_[coro_id];

	auto& ucontext = coro->ucontext();
	switch (coro->status()) {
	case CoroStatus::CORO_STATUS_READY:
		{
			::getcontext(&ucontext);
			ucontext.uc_stack.ss_sp = stack_;
			ucontext.uc_stack.ss_size = kStackSize;
			ucontext.uc_link = &main_;
			coro->set_status(CoroStatus::CORO_STATUS_RUNNING);
			running_id_ = coro_id;

			auto sched_ptr = (uintptr_t)this;
			::makecontext(&ucontext, (void (*)())start_func, 2, (uint32_t)sched_ptr, (uint32_t)(sched_ptr >> 32));
			::swapcontext(&main_, &ucontext);
		} break;
	case CoroStatus::CORO_STATUS_SUSPEND:
		{
			::memcpy(stack_ + kStackSize - coro->size(), coro->stack(), coro->size());
			coro->set_status(CoroStatus::CORO_STATUS_RUNNING);
			running_id_ = coro_id;
			::swapcontext(&main_, &ucontext);
		} break;
	default: break;
	}
}

void Schedule::coroutine_yield() noexcept {
	int coro_id = running_id_;
	Coroutine* coro = coro_list_[coro_id];

	coro->set_status(CoroStatus::CORO_STATUS_SUSPEND);
	save_stack(coro, stack_ + kStackSize);
	running_id_ = -1;

	auto& ucontext = coro->ucontext();
	::swapcontext(&ucontext, &main_);
}

CoroStatus Schedule::get_coroutine_status(int coro_id) const noexcept {
	Coroutine* coro = coro_list_[coro_id];
	if (coro == nullptr)
		return CoroStatus::CORO_STATUS_DEAD;
	return coro->status();
}

int Schedule::get_running_coroutine_id() const noexcept {
	return running_id_;
}

void Schedule::save_stack(Coroutine* coro, const char* top) noexcept {
	char dummy = 0;
	if (coro->cap() < top - &dummy) {
		coro->set_cap(top - &dummy);
		coro->acquire();
	}
	coro->set_size(top - &dummy);
	coro->set_stack(&dummy);
}

void Schedule::start_running_coroutine() noexcept {
	Coroutine* coro = coro_list_[running_id_];
	coro->start(*this);
	coro->release();

	--coro_num_;
	coro_list_[running_id_] = nullptr;
	running_id_ = -1;
}

}
