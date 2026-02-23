import test from "node:test";
import assert from "node:assert/strict";
import { APP_VERSION, formatVersionOutput, resolveAppVersion } from "../../src/version";

test("version string follows semver-like format", () => {
  assert.match(APP_VERSION, /^\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?$/);
});

test("version output is non-empty", () => {
  const output = formatVersionOutput().trim();
  assert.notEqual(output.length, 0);
});

test("resolves app version from injected environment value", () => {
  const version = resolveAppVersion({ MINICLI_APP_VERSION: "9.9.9" });
  assert.equal(version, "9.9.9");
});
