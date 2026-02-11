import test from "node:test";
import assert from "node:assert/strict";
import http from "node:http";
import { AddressInfo } from "node:net";
import { GLMOpenAIProvider } from "../../src/provider";

test("GLM provider succeeds against mock OpenAI-compatible API", async () => {
  let requestUrl = "";
  let requestAuth = "";
  let requestBody = "";

  const server = http.createServer((req, res) => {
    requestUrl = req.url ?? "";
    requestAuth = String(req.headers.authorization ?? "");

    const chunks: Buffer[] = [];
    req.on("data", (chunk: Buffer) => chunks.push(chunk));
    req.on("end", () => {
      requestBody = Buffer.concat(chunks).toString("utf8");
      res.writeHead(200, { "Content-Type": "application/json" });
      res.end(
        JSON.stringify({
          choices: [
            {
              message: {
                role: "assistant",
                content: "mock-api reply"
              }
            }
          ]
        })
      );
    });
  });

  await new Promise<void>((resolve, reject) => {
    server.once("error", reject);
    server.listen(0, "127.0.0.1", () => resolve());
  });

  try {
    const address = server.address() as AddressInfo;
    const provider = new GLMOpenAIProvider({
      apiKey: "test-key",
      baseUrl: `http://127.0.0.1:${address.port}`
    });

    const response = await provider.complete({
      model: "glm-4",
      timeoutMs: 5000,
      messages: [{ role: "user", content: "hello api" }]
    });

    assert.equal(response.message.role, "assistant");
    assert.equal(response.message.content, "mock-api reply");
    assert.equal(requestUrl, "/chat/completions");
    assert.equal(requestAuth, "Bearer test-key");
    const parsedBody = JSON.parse(requestBody) as {
      model: string;
      messages: Array<{ role: string; content: string }>;
    };
    assert.equal(parsedBody.model, "glm-4");
    assert.deepEqual(parsedBody.messages, [{ role: "user", content: "hello api" }]);
  } finally {
    await new Promise<void>((resolve, reject) => {
      server.close((error) => {
        if (error) {
          reject(error);
          return;
        }
        resolve();
      });
    });
  }
});

test("GLM provider retries 429 and then succeeds", async () => {
  let requestCount = 0;

  const server = http.createServer((_req, res) => {
    requestCount += 1;
    if (requestCount === 1) {
      res.writeHead(429, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: "rate limited" }));
      return;
    }

    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(
      JSON.stringify({
        choices: [
          {
            message: {
              role: "assistant",
              content: "retry success"
            }
          }
        ]
      })
    );
  });

  await new Promise<void>((resolve, reject) => {
    server.once("error", reject);
    server.listen(0, "127.0.0.1", () => resolve());
  });

  try {
    const address = server.address() as AddressInfo;
    const provider = new GLMOpenAIProvider({
      apiKey: "test-key",
      baseUrl: `http://127.0.0.1:${address.port}`,
      maxRetries: 2,
      retryDelayMs: 0
    });

    const response = await provider.complete({
      model: "glm-4",
      timeoutMs: 5000,
      messages: [{ role: "user", content: "hello retry" }]
    });

    assert.equal(response.message.content, "retry success");
    assert.equal(requestCount, 2);
  } finally {
    await new Promise<void>((resolve, reject) => {
      server.close((error) => {
        if (error) {
          reject(error);
          return;
        }
        resolve();
      });
    });
  }
});
