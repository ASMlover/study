import { RuntimeConfig } from "./config";
import { ansi } from "./ansi";

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
  private streamOpen = false;
  private turn = 0;
  private lastStatusSignature = "";
  private thinkingTimer: NodeJS.Timeout | null = null;
  private thinkingFrame = 0;
  private thinkingActive = false;
  private readonly thinkingFrames = [
    "[thinking] ⠋ planning",
    "[thinking] ⠙ planning",
    "[thinking] ⠹ reasoning",
    "[thinking] ⠸ reasoning",
    "[thinking] ⠼ drafting",
    "[thinking] ⠴ drafting",
    "[thinking] ⠦ refining",
    "[thinking] ⠧ refining",
    "[thinking] ⠇ finalizing",
    "[thinking] ⠏ finalizing"
  ];

  constructor(private readonly output: NodeJS.WritableStream & { isTTY?: boolean; columns?: number }) {
    this.isTTY = Boolean(output.isTTY);
  }

  start(status: PaneState): void {
    if (!this.isTTY) {
      return;
    }
    this.output.write(`${ansi.bold(ansi.cyan("MiniCLI4 TUI"))} ${ansi.gray("(append-only)")}\n`);
    this.output.write(`${ansi.gray("keys:")} ${ansi.blue("Tab")} next/prev | ${ansi.blue("Ctrl+C")} stop | ${ansi.blue("Ctrl+L")} status | ${ansi.blue("F1")} /help | ${ansi.blue("Esc")} close menu\n`);
    this.lastStatusSignature = this.signature(status);
  }

  announceInput(input: string): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    this.turn += 1;
    this.endAssistantStream();
    this.output.write(`\n${ansi.gray(`[${String(this.turn).padStart(2, "0")}]`)} ${ansi.green(">")} ${ansi.bold(input)}\n`);
  }

  printEvent(line: string): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    this.endAssistantStream();
    this.output.write(`${ansi.gray("•")} ${line}\n`);
  }

  updateStatus(status: PaneState): void {
    if (!this.isTTY) {
      return;
    }
    this.lastStatusSignature = this.signature(status);
  }

  printStatus(status: PaneState): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    this.lastStatusSignature = this.signature(status);
    this.endAssistantStream();
    this.output.write(`${ansi.yellow("[status]")} session=${ansi.cyan(status.sessionId)} model=${ansi.magenta(status.model)} stage=${ansi.blue(status.stage)} mode=${ansi.green(status.mode)} approval=${ansi.yellow(status.pendingApproval)}\n`);
  }

  startAssistantStream(): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    if (this.streamOpen) {
      return;
    }
    this.streamOpen = true;
    this.output.write(`${ansi.magenta("✦")} `);
  }

  appendAssistantChunk(chunk: string): void {
    if (!this.isTTY) {
      return;
    }
    if (!this.streamOpen) {
      this.startAssistantStream();
    }
    this.output.write(chunk);
  }

  endAssistantStream(): void {
    if (!this.isTTY || !this.streamOpen) {
      return;
    }
    this.output.write("\n");
    this.streamOpen = false;
  }

  startThinking(): void {
    if (!this.isTTY || this.thinkingActive) {
      return;
    }
    this.endAssistantStream();
    this.thinkingActive = true;
    this.thinkingFrame = 0;
    this.output.write(`\r\x1b[2K${ansi.blue(this.thinkingFrames[this.thinkingFrame])}`);
    this.thinkingTimer = setInterval(() => {
      if (!this.thinkingActive) {
        return;
      }
      this.thinkingFrame = (this.thinkingFrame + 1) % this.thinkingFrames.length;
      this.output.write(`\r\x1b[2K${ansi.blue(this.thinkingFrames[this.thinkingFrame])}`);
    }, 120);
  }

  stopThinking(): void {
    if (!this.isTTY || !this.thinkingActive) {
      return;
    }
    if (this.thinkingTimer) {
      clearInterval(this.thinkingTimer);
      this.thinkingTimer = null;
    }
    this.thinkingActive = false;
    this.output.write("\r\x1b[2K");
  }

  printLine(text: string): void {
    this.output.write(`${text}\n`);
  }

  applyConfig(_config: RuntimeConfig): void {
    // reserved for future style/runtime changes
  }

  private signature(status: PaneState): string {
    return `${status.sessionId}|${status.model}|${status.stage}|${status.mode}|${status.pendingApproval}`;
  }
}
