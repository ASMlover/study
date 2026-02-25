import test from "node:test";
import assert from "node:assert/strict";
import { runCli } from "../../src/cli";

test("cli version flag works", () => {
  let out = "";
  const orig = process.stdout.write.bind(process.stdout);
  (process.stdout.write as unknown as (chunk: string) => boolean) = ((chunk: string) => {
    out += chunk;
    return true;
  }) as unknown as (chunk: string) => boolean;
  try {
    const code = runCli(["--version"]);
    assert.equal(code, 0);
    assert.match(out, /0.1.0/);
  } finally {
    process.stdout.write = orig;
  }
});

test("cli invalid arg returns non-zero", () => {
  let err = "";
  const orig = process.stderr.write.bind(process.stderr);
  (process.stderr.write as unknown as (chunk: string) => boolean) = ((chunk: string) => {
    err += chunk;
    return true;
  }) as unknown as (chunk: string) => boolean;
  try {
    const code = runCli(["unknown"]);
    assert.equal(code, 1);
    assert.match(err, /Unknown argument/);
  } finally {
    process.stderr.write = orig;
  }
});
