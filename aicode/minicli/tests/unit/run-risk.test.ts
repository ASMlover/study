import test from "node:test";
import assert from "node:assert/strict";
import { classifyRunCommandRisk } from "../../src/run-risk";

test("classifyRunCommandRisk marks simple read command as low risk", () => {
  const result = classifyRunCommandRisk("pwd");
  assert.equal(result.level, "low");
  assert.deepEqual(result.matchedRules, []);
});

test("classifyRunCommandRisk marks command with shell operators as medium risk", () => {
  const result = classifyRunCommandRisk("pwd | wc -c");
  assert.equal(result.level, "medium");
  assert.match(result.matchedRules.join(","), /\|/);
});

test("classifyRunCommandRisk marks destructive command as high risk", () => {
  const result = classifyRunCommandRisk("rm -rf .");
  assert.equal(result.level, "high");
  assert.match(result.matchedRules.join(","), /rm/);
});

test("classifyRunCommandRisk keeps highest risk for mixed command", () => {
  const result = classifyRunCommandRisk("pwd && rm -rf .");
  assert.equal(result.level, "high");
  assert.match(result.matchedRules.join(","), /rm/);
});

test("classifyRunCommandRisk is resilient to whitespace and case", () => {
  const result = classifyRunCommandRisk("   ReBoOt   ");
  assert.equal(result.level, "high");
  assert.match(result.matchedRules.join(","), /reboot/i);
});

test("classifyRunCommandRisk catches bypass-style separators", () => {
  const result = classifyRunCommandRisk("r`m -rf .");
  assert.equal(result.level, "high");
  assert.match(result.matchedRules.join(","), /rm/);
});
