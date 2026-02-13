import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";
import { LLMProvider } from "../../src/provider";

test("chat request includes assembled context snippets", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-context-req-"));
  const prevCwd = process.cwd();
  try {
    fs.writeFileSync(path.join(tmpRoot, "guide.md"), "first\r\nsecond\r\n", "utf8");
    process.chdir(tmpRoot);

    let capturedSystem = "";
    const provider: LLMProvider = {
      id: "capture-context-snippet",
      complete: async (request) => {
        capturedSystem =
          request.messages.find((message) => message.role === "system")?.content ?? "";
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
      { provider }
    );

    await session.onLine("/add guide.md");
    await session.onLine("hello");

    assert.equal(errors.join(""), "");
    assert.match(capturedSystem, /^Context snippets:/);
    assert.match(capturedSystem, /\[1\] path: guide\.md/);
    assert.match(capturedSystem, /\[1\] meta: lines=3, chars=13, encoding=utf-8/);
    assert.match(capturedSystem, /```text\nfirst\nsecond\n```/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
