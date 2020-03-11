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
#include <iostream>
#include "../common.hh"
#include "../harness.hh"

// Question: https://leetcode.com/problems/reverse-linked-list/description/
//
// Reverse a singly linked list
// Example:
//    Input:  1->2->3->4->5->null
//    Output: 5->4->3->2->1->null
// follow up:
// A linked list can be reversed either iterativly or recursively, could you
// implement both ?

class ReverseLinkedList final : public Singleton<ReverseLinkedList> {
  struct ListNode {
    ListNode* next{};
    int value{};

    ListNode(int v = 0) noexcept : value(v) {}
  };

  void show(ListNode* head) noexcept {
    std::cout << "show list: " << head << std::endl;
    for (auto* n = head; n != nullptr; n = n->next)
      std::cout << "\tlist node value is: " << n->value << std::endl;
  }

  void destroy(ListNode* head) {
    while (head != nullptr) {
      ListNode* node = head;
      head = head->next;
      delete node;
    }
  }

  ListNode* append(ListNode* head, int value) {
    ListNode* node = new ListNode(value);
    node->next = head;
    head = node;

    return head;
  }

  ListNode* reverse_iteratively(ListNode* head) {
    // prev记录节点的前驱，next记录节点的后继，不停更新current->next = prev

    ListNode* curr{head};
    ListNode* prev{};
    ListNode* next{};
    while (curr != nullptr) {
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
    return prev;
  }

  ListNode* reverse_recursively(ListNode* head) {
    // current节点或next节点为null直接返回，否则prev为next的递归调用返回的节点
    // 同时将next的next设置为当前节点，当前节点的next设置为null

    if (head == nullptr || head->next == nullptr)
      return head;

    ListNode* next = head->next;
    ListNode* prev = reverse_recursively(next);
    next->next = head;
    head->next = nullptr;

    return prev;
  }
public:
  void run() {
    ListNode* head{};
    for (int i = 0; i < 5; ++i)
      head = append(head, i + 1);
    show(head);

    head = reverse_iteratively(head);
    show(head);

    head = reverse_recursively(head);
    show(head);

    destroy(head);
  }
};

HARNESS_TEST(ReverseLinkedList, harness::FakeTester) {
  ReverseLinkedList::get_instance().run();
}
