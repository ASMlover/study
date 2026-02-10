import test from "node:test";
import assert from "node:assert/strict";
import { APP_VERSION, formatVersionOutput } from "../../src/version";

test("version string follows semver-like format", () => {
  assert.match(APP_VERSION, /^\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?$/);
});

test("version output is non-empty", () => {
  const output = formatVersionOutput().trim();
  assert.notEqual(output.length, 0);
});
