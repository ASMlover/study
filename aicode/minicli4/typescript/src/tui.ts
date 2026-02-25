import { RuntimeConfig } from "./config";

export interface PaneState {
  sessionId: string;
  model: string;
  stage: string;
  tools: string;
  mode: string;
  pendingApproval: string;
}

export class TwoPaneTui {
  private readonly isTTY: boolean;
  constructor(private readonly output: NodeJS.WritableStream & { isTTY?: boolean; columns?: number }) {
    this.isTTY = Boolean(output.isTTY);
  }

  render(messages: string[], status: PaneState): void {
    if (!this.isTTY) {
      return;
    }
    const width = this.output.columns ?? 120;
    const leftWidth = Math.max(50, Math.floor(width * 0.7));
    const rightWidth = Math.max(24, width - leftWidth - 3);

    const left = messages.slice(-18).map((line) => fit(line, leftWidth));
    const right = [
      `session: ${status.sessionId}`,
      `model: ${status.model}`,
      `stage: ${status.stage}`,
      `tools: ${status.tools}`,
      `mode: ${status.mode}`,
      `approval: ${status.pendingApproval}`,
      "",
      "keys:",
      "Tab/Shift+Tab complete",
      "Ctrl+C interrupt",
      "Ctrl+L clear screen",
      "F1 help",
      "Esc hide candidates"
    ].map((line) => fit(line, rightWidth));

    const rows = Math.max(left.length, right.length, 16);
    const out: string[] = ["\x1b[2J\x1b[H", `MiniCLI4 TUI ${"-".repeat(Math.max(0, width - 13))}`];
    for (let i = 0; i < rows; i += 1) {
      const l = (left[i] ?? "").padEnd(leftWidth, " ");
      const r = right[i] ?? "";
      out.push(`${l} | ${r}`);
    }
    out.push("-".repeat(width));
    this.output.write(`${out.join("\n")}\n`);
  }

  printLine(text: string): void {
    this.output.write(`${text}\n`);
  }

  applyConfig(_config: RuntimeConfig): void {
    // reserved for future style/runtime changes
  }
}

function fit(text: string, width: number): string {
  if (text.length <= width) {
    return text;
  }
  if (width <= 1) {
    return text.slice(0, width);
  }
  return `${text.slice(0, width - 1)}...`;
}
