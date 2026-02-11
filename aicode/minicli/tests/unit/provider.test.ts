import test from "node:test";
import assert from "node:assert/strict";
import {
  buildChatRequest,
  ChatRequest,
  LLMProvider,
  MockLLMProvider
} from "../../src/provider";

test("provider interface contract can be implemented", async () => {
  const provider: LLMProvider = new MockLLMProvider();
  const request: ChatRequest = {
    model: "mock-mini",
    timeoutMs: 1000,
    messages: [{ role: "user", content: "hello" }]
  };
  const response = await provider.complete(request);

  assert.equal(provider.id, "mock");
  assert.equal(response.message.role, "assistant");
});

test("buildChatRequest shapes request with history and latest user message", () => {
  const request = buildChatRequest(
    [{ role: "assistant", content: "prev" }],
    "next",
    { model: "glm-4", timeoutMs: 2000 }
  );

  assert.equal(request.model, "glm-4");
  assert.equal(request.timeoutMs, 2000);
  assert.deepEqual(request.messages, [
    { role: "assistant", content: "prev" },
    { role: "user", content: "next" }
  ]);
});

test("mock provider response shape is assistant message", async () => {
  const provider = new MockLLMProvider();
  const response = await provider.complete({
    model: "mock-mini",
    timeoutMs: 2000,
    messages: [{ role: "user", content: "question" }]
  });

  assert.equal(response.message.role, "assistant");
  assert.match(response.message.content, /^mock\(mock-mini\): question$/);
});

test("mock provider propagates error for empty message", async () => {
  const provider = new MockLLMProvider();

  await assert.rejects(
    provider.complete({
      model: "mock-mini",
      timeoutMs: 2000,
      messages: [{ role: "user", content: "   " }]
    }),
    /No user message provided/
  );
});

test("provider exceptions propagate to caller", async () => {
  const provider: LLMProvider = {
    id: "boom",
    complete: async () => {
      throw new Error("boom");
    }
  };

  await assert.rejects(
    provider.complete({
      model: "mock-mini",
      timeoutMs: 1,
      messages: [{ role: "user", content: "x" }]
    }),
    /boom/
  );
});

test("multi-turn assembly keeps prior turns when building request", () => {
  const history = [
    { role: "user" as const, content: "u1" },
    { role: "assistant" as const, content: "a1" }
  ];

  const request = buildChatRequest(history, "u2", {
    model: "mock-mini",
    timeoutMs: 30000
  });

  assert.equal(request.messages.length, 3);
  assert.deepEqual(request.messages[0], { role: "user", content: "u1" });
  assert.deepEqual(request.messages[1], { role: "assistant", content: "a1" });
  assert.deepEqual(request.messages[2], { role: "user", content: "u2" });
});
