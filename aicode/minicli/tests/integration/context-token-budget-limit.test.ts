import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import {
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  estimateChatRequestTokens
} from "../../src/repl";
import { LLMProvider } from "../../src/provider";

test("long context is trimmed to fit request token budget", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-token-budget-"));
  const prevCwd = process.cwd();
  try {
    const longText = "line-1234567890".repeat(200);
    fs.writeFileSync(path.join(tmpRoot, "large.md"), longText, "utf8");
    process.chdir(tmpRoot);

    let capturedTokens = 0;
    const provider: LLMProvider = {
      id: "capture-token-budget",
      complete: async (request) => {
        capturedTokens = estimateChatRequestTokens(request.messages);
        return {
          message: {
            role: "assistant",
            content: "ok"
          }
        };
      }
    };

    const writes: string[] = [];
    const errors: string[] = [];
    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH,
      {
        provider,
        requestTokenBudget: 30
      }
    );

    await session.onLine("/add large.md");
    await session.onLine("hello");

    assert.equal(capturedTokens <= 30, true);
    assert.match(errors.join(""), /\[context:warn\] request exceeded token budget 30/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
