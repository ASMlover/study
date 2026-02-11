import test from "node:test";
import assert from "node:assert/strict";
import {
  buildGLMHeaders,
  buildChatRequest,
  buildOpenAICompatibleUrl,
  ChatRequest,
  GLMOpenAIProvider,
  LLMProvider,
  mapOpenAICompatibleResponse,
  MockLLMProvider,
  ProviderNetworkError
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

test("GLM provider request headers include auth and content type", () => {
  const headers = buildGLMHeaders("secret-key");
  assert.equal(headers.Authorization, "Bearer secret-key");
  assert.equal(headers["Content-Type"], "application/json");
});

test("GLM provider URL join is OpenAI-compatible", () => {
  assert.equal(
    buildOpenAICompatibleUrl("https://open.bigmodel.cn/api/paas/v4/"),
    "https://open.bigmodel.cn/api/paas/v4/chat/completions"
  );
});

test("GLM provider falls back to default model when request model is empty", async () => {
  let capturedBody = "";
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    defaultModel: "glm-4-air",
    fetchImpl: (async (_input: string | URL | Request, init?: RequestInit) => {
      capturedBody = String(init?.body ?? "");
      return new Response(
        JSON.stringify({
          choices: [{ message: { role: "assistant", content: "ok" } }]
        }),
        {
          status: 200,
          headers: { "Content-Type": "application/json" }
        }
      );
    }) as typeof fetch
  });

  await provider.complete({
    model: "",
    timeoutMs: 1000,
    messages: [{ role: "user", content: "hello" }]
  });

  const payload = JSON.parse(capturedBody) as { model: string };
  assert.equal(payload.model, "glm-4-air");
});

test("GLM provider maps OpenAI-compatible response", () => {
  const mapped = mapOpenAICompatibleResponse({
    choices: [{ message: { role: "assistant", content: "mapped" } }]
  });

  assert.deepEqual(mapped, {
    message: { role: "assistant", content: "mapped" }
  });
});

test("GLM provider throws for empty choices", async () => {
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    fetchImpl: (async () =>
      new Response(JSON.stringify({ choices: [] }), {
        status: 200,
        headers: { "Content-Type": "application/json" }
      })) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 1000,
      messages: [{ role: "user", content: "hello" }]
    }),
    /No choices returned by provider/
  );
});

test("GLM provider wraps network errors", async () => {
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 0,
    fetchImpl: (async () => {
      throw new Error("connect ECONNREFUSED");
    }) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 1000,
      messages: [{ role: "user", content: "hello" }]
    }),
    /GLM network error/
  );
});

test("GLM provider maps timeout error", async () => {
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 0,
    fetchImpl: ((_input: string | URL | Request, init?: RequestInit) =>
      new Promise<Response>((_resolve, reject) => {
        init?.signal?.addEventListener("abort", () => {
          const abortError = new Error("The operation was aborted.");
          abortError.name = "AbortError";
          reject(abortError);
        });
      })) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 20,
      messages: [{ role: "user", content: "hello" }]
    }),
    /timed out/
  );
});

test("GLM provider maps 401 without retry", async () => {
  let attempts = 0;
  const provider = new GLMOpenAIProvider({
    apiKey: "bad-key",
    maxRetries: 3,
    retryDelayMs: 0,
    fetchImpl: (async () => {
      attempts += 1;
      return new Response("unauthorized", { status: 401 });
    }) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 100,
      messages: [{ role: "user", content: "hello" }]
    }),
    (error: unknown) => {
      assert.ok(error instanceof ProviderNetworkError);
      assert.equal(error.code, "http_401");
      return true;
    }
  );
  assert.equal(attempts, 1);
});

test("GLM provider maps 429 to retryable error", async () => {
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 0,
    fetchImpl: (async () => new Response("too many requests", { status: 429 })) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 100,
      messages: [{ role: "user", content: "hello" }]
    }),
    (error: unknown) => {
      assert.ok(error instanceof ProviderNetworkError);
      assert.equal(error.code, "http_429");
      assert.equal(error.retryable, true);
      return true;
    }
  );
});

test("GLM provider maps 5xx to retryable error", async () => {
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 0,
    fetchImpl: (async () => new Response("server error", { status: 503 })) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 100,
      messages: [{ role: "user", content: "hello" }]
    }),
    (error: unknown) => {
      assert.ok(error instanceof ProviderNetworkError);
      assert.equal(error.code, "http_5xx");
      assert.equal(error.retryable, true);
      return true;
    }
  );
});

test("GLM provider retries retryable status until success", async () => {
  let attempts = 0;
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 2,
    retryDelayMs: 0,
    fetchImpl: (async () => {
      attempts += 1;
      if (attempts < 3) {
        return new Response("too many requests", { status: 429 });
      }
      return new Response(
        JSON.stringify({
          choices: [{ message: { role: "assistant", content: "ok after retry" } }]
        }),
        { status: 200, headers: { "Content-Type": "application/json" } }
      );
    }) as typeof fetch
  });

  const response = await provider.complete({
    model: "glm-4",
    timeoutMs: 100,
    messages: [{ role: "user", content: "hello" }]
  });

  assert.equal(response.message.content, "ok after retry");
  assert.equal(attempts, 3);
});

test("GLM provider does not retry non-retryable status", async () => {
  let attempts = 0;
  const provider = new GLMOpenAIProvider({
    apiKey: "k",
    maxRetries: 5,
    retryDelayMs: 0,
    fetchImpl: (async () => {
      attempts += 1;
      return new Response("unauthorized", { status: 401 });
    }) as typeof fetch
  });

  await assert.rejects(
    provider.complete({
      model: "glm-4",
      timeoutMs: 100,
      messages: [{ role: "user", content: "hello" }]
    }),
    /Authentication failed/
  );
  assert.equal(attempts, 1);
});
