import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";
import { LLMProvider } from "../../src/provider";

test("after /drop, removed file is excluded from next chat request context", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-context-drop-"));
  const prevCwd = process.cwd();
  try {
    const keepFile = path.join(tmpRoot, "keep.md");
    const dropFile = path.join(tmpRoot, "drop.md");
    fs.writeFileSync(keepFile, "# keep\n", "utf8");
    fs.writeFileSync(dropFile, "# drop\n", "utf8");
    process.chdir(tmpRoot);

    let capturedContext = "";
    const provider: LLMProvider = {
      id: "capture-context",
      complete: async (request) => {
        capturedContext =
          request.messages.find((message) => message.role === "system")?.content ??
          "";
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

    await session.onLine("/add keep.md");
    await session.onLine("/add drop.md");
    await session.onLine("/drop drop.md");
    await session.onLine("hello");

    assert.equal(errors.join(""), "");
    assert.match(capturedContext, /keep\.md/);
    assert.doesNotMatch(capturedContext, /drop\.md/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
