import test from "node:test";
import assert from "node:assert/strict";
import { complete } from "../../src/completion";

test("completion supports command prefix", () => {
  const out = complete("/co", {
    commandNames: ["/config", "/compact", "/context", "/help"],
    sessionIds: ["default"],
    configKeys: ["api_key", "timeout_ms"],
    toolNames: ["read_file", "run_shell"],
    projectRoot: process.cwd()
  });
  assert.deepEqual(out, ["/compact", "/config", "/context"]);
});

test("completion supports dynamic config keys", () => {
  const out = complete("/config set ", {
    commandNames: ["/config"],
    sessionIds: ["default"],
    configKeys: ["api_key", "max_tokens"],
    toolNames: [],
    projectRoot: process.cwd()
  });
  assert.deepEqual(out, ["api_key", "max_tokens"]);
});
