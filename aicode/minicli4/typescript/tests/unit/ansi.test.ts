import test from "node:test";
import assert from "node:assert/strict";
import { ansi, detectAnsiTheme, getAnsiTheme, setAnsiTheme, stripAnsi } from "../../src/ansi";

test("detectAnsiTheme respects explicit MINICLI4_THEME", () => {
  assert.equal(detectAnsiTheme({ MINICLI4_THEME: "light" }), "light");
  assert.equal(detectAnsiTheme({ MINICLI4_THEME: "dark" }), "dark");
});

test("detectAnsiTheme infers from COLORFGBG background", () => {
  assert.equal(detectAnsiTheme({ COLORFGBG: "15;0" }), "dark");
  assert.equal(detectAnsiTheme({ COLORFGBG: "0;15" }), "light");
});

test("setAnsiTheme changes active palette", () => {
  const before = getAnsiTheme();
  setAnsiTheme("light");
  assert.equal(getAnsiTheme(), "light");
  const rendered = ansi.code("sample");
  assert.equal(stripAnsi(rendered), "sample");
  setAnsiTheme(before);
});
