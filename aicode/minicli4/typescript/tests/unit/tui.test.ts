import test from "node:test";
import assert from "node:assert/strict";
import { Writable } from "node:stream";
import { PaneState, TwoPaneTui } from "../../src/tui";
import { stripAnsi } from "../../src/ansi";

function makePane(overrides?: Partial<PaneState>): PaneState {
  return {
    sessionId: "default",
    model: "glm-5",
    stage: "idle",
    tools: "run_shell,read_file,list_dir,write_file,session_export,project_tree,grep_text",
    mode: "strict",
    pendingApproval: "none",
    ...overrides
  };
}

test("tui uses append-only output without full-screen clear", () => {
  const chunks: string[] = [];
  const output = new Writable({
    write(chunk, _encoding, callback) {
      chunks.push(String(chunk));
      callback();
    }
  }) as Writable & { isTTY?: boolean; columns?: number };
  output.isTTY = true;
  output.columns = 100;

  const tui = new TwoPaneTui(output);
  tui.start(makePane());
  tui.announceInput("/help");
  tui.printEvent("MiniCLI4 Help");
  tui.updateStatus(makePane({ stage: "planner" }));
  tui.printStatus(makePane({ stage: "planner" }));
  tui.startThinking();
  tui.stopThinking();
  tui.startAssistantStream();
  tui.appendAssistantChunk("Hello");
  tui.appendAssistantChunk(" world");
  tui.endAssistantStream();

  const text = chunks.join("");
  const plain = stripAnsi(text);
  assert.doesNotMatch(text, /\x1b\[2J/);
  assert.match(plain, /MiniCLI4 TypeScript Agent CLI/);
  assert.match(plain, /\[\d{2}:\d{2}:\d{2}\] \[YOU 01\] \/help/);
  assert.match(plain, /MiniCLI4 Help/);
  assert.match(plain, /\[STATUS\] session=default model=glm-5 stage=planner mode=strict approval=none/);
  assert.match(plain, /(Thinking|Reasoning|Drafting|Refining|Finalizing)/);
  assert.match(plain, /MiniCLI4/);
  assert.match(plain, /(🧠|🧭|🛠|✨|🚀)/);
  assert.match(plain, /Hello world/);
});

test("updateStatus is silent and printStatus is explicit", () => {
  const chunks: string[] = [];
  const output = new Writable({
    write(chunk, _encoding, callback) {
      chunks.push(String(chunk));
      callback();
    }
  }) as Writable & { isTTY?: boolean; columns?: number };
  output.isTTY = true;

  const tui = new TwoPaneTui(output);
  const pane = makePane();
  tui.start(pane);
  tui.updateStatus(pane);
  tui.updateStatus(makePane({ stage: "planner" }));
  tui.printStatus(makePane({ stage: "planner" }));

  const text = stripAnsi(chunks.join(""));
  const statusCount = (text.match(/\[STATUS\]/g) ?? []).length;
  assert.equal(statusCount, 1);
});
