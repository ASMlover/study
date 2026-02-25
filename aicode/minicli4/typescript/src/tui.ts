import { RuntimeConfig } from "./config";
import { ansi, getAnsiTheme } from "./ansi";

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
  private motion: "full" | "minimal" = "full";
  private turn = 0;
  private lastStatusSignature = "";
  private thinkingTimer: NodeJS.Timeout | null = null;
  private thinkingFrame = 0;
  private thinkingActive = false;
  private thinkingStartedAt = 0;
  private readonly thinkingLogoFrames = ["◉ MiniCLI4 ◉", "◎ MiniCLI4 ◉", "◎ MiniCLI4 ◎", "◉ MiniCLI4 ◎"];
  private readonly thinkingAuraFrames = ["✦", "✧", "✨", "✧"];
  private readonly thinkingStages = [
    { name: "Thinking", icon: "🧠", details: ["collecting context", "reviewing intent", "loading constraints"] },
    { name: "Reasoning", icon: "🧭", details: ["evaluating options", "checking tradeoffs", "validating assumptions"] },
    { name: "Drafting", icon: "🛠", details: ["structuring response", "building answer", "assembling details"] },
    { name: "Refining", icon: "✨", details: ["improving clarity", "tightening phrasing", "verifying coherence"] },
    { name: "Finalizing", icon: "🚀", details: ["preparing stream", "readying output", "sending response"] }
  ];

  constructor(private readonly output: NodeJS.WritableStream & { isTTY?: boolean; columns?: number }) {
    this.isTTY = Boolean(output.isTTY);
  }

  start(status: PaneState): void {
    if (!this.isTTY) {
      return;
    }
    const theme = getAnsiTheme();
    this.output.write(`${ansi.bold(ansi.cyan("MiniCLI4"))} ${ansi.gray("TypeScript Agent CLI")}` + "\n");
    this.output.write(`${ansi.gray("Mode:")} ${ansi.green(status.mode)}  ${ansi.gray("Model:")} ${ansi.magenta(status.model)}  ${ansi.gray("Theme:")} ${ansi.yellow(theme)}  ${ansi.gray("Motion:")} ${ansi.yellow(this.motion)}  ${ansi.gray("Session:")} ${ansi.cyan(status.sessionId)}\n`);
    this.output.write(`${ansi.gray("Keys:")} ${ansi.blue("Tab")} complete | ${ansi.blue("Ctrl+C")} stop | ${ansi.blue("Ctrl+L")} status | ${ansi.blue("F1")} help | ${ansi.blue("Esc")} close menu\n`);
    this.output.write(`${ansi.gray("-".repeat(72))}\n`);
    this.lastStatusSignature = this.signature(status);
  }

  announceInput(input: string): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    this.turn += 1;
    this.endAssistantStream();
    const stamp = nowTime();
    this.output.write(`\n${ansi.gray(`[${stamp}]`)} ${ansi.green(`[YOU ${String(this.turn).padStart(2, "0")}]`)} ${ansi.bold(input)}\n`);
  }

  printEvent(line: string): void {
    if (!this.isTTY) {
      return;
    }
    this.stopThinking();
    this.endAssistantStream();
    this.output.write(`${line}\n`);
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
    this.output.write(
      `${ansi.yellow("[STATUS]")} ` +
      `${ansi.gray("session=")}${ansi.cyan(status.sessionId)} ` +
      `${ansi.gray("model=")}${ansi.magenta(status.model)} ` +
      `${ansi.gray("stage=")}${ansi.blue(status.stage)} ` +
      `${ansi.gray("mode=")}${ansi.green(status.mode)} ` +
      `${ansi.gray("approval=")}${ansi.yellow(status.pendingApproval)}\n`
    );
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
    this.thinkingStartedAt = Date.now();
    this.output.write(`\r\x1b[2K${this.renderThinkingFrame()}`);
    const intervalMs = this.motion === "minimal" ? 480 : 160;
    this.thinkingTimer = setInterval(() => {
      if (!this.thinkingActive) {
        return;
      }
      this.thinkingFrame += 1;
      this.output.write(`\r\x1b[2K${this.renderThinkingFrame()}`);
    }, intervalMs);
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

  applyConfig(config: RuntimeConfig): void {
    this.motion = config.motion;
  }

  private signature(status: PaneState): string {
    return `${status.sessionId}|${status.model}|${status.stage}|${status.mode}|${status.pendingApproval}`;
  }

  private renderThinkingFrame(): string {
    const elapsedMs = Date.now() - this.thinkingStartedAt;
    const elapsedSec = Math.floor(elapsedMs / 1000);
    const mm = String(Math.floor(elapsedSec / 60)).padStart(2, "0");
    const ss = String(elapsedSec % 60).padStart(2, "0");

    const stage = this.thinkingStages[Math.floor(this.thinkingFrame / 6) % this.thinkingStages.length];
    const detail = stage.details[this.thinkingFrame % stage.details.length];
    const logo = ansi.cyan(this.thinkingLogoFrames[this.thinkingFrame % this.thinkingLogoFrames.length]);
    const aura = ansi.yellow(this.thinkingAuraFrames[this.thinkingFrame % this.thinkingAuraFrames.length]);
    const stageIcon = ansi.magenta(stage.icon);
    const stageColor = this.thinkingFrame % 2 === 0 ? ansi.blue : ansi.magenta;
    const stageText = stageColor(stage.name.padEnd(10, " "));
    const timer = ansi.gray(`${mm}:${ss}`);
    const detailText = this.motion === "minimal" ? ansi.gray(detail) : ansi.gray(animateDetail(detail, this.thinkingFrame));
    if (this.motion === "minimal") {
      return `${logo} ${stageIcon} ${stageText} ${timer} ${detailText}`;
    }
    return `${logo} ${aura} ${stageIcon} ${stageText} ${timer} ${aura} ${detailText}`;
  }
}

function animateDetail(detail: string, frame: number): string {
  const words = detail.split(" ");
  if (words.length <= 1) {
    return detail;
  }
  const hi = frame % words.length;
  return words
    .map((word, idx) => (idx === hi ? ansi.bold(word) : word))
    .join(" ");
}

function nowTime(): string {
  const d = new Date();
  const hh = String(d.getHours()).padStart(2, "0");
  const mm = String(d.getMinutes()).padStart(2, "0");
  const ss = String(d.getSeconds()).padStart(2, "0");
  return `${hh}:${mm}:${ss}`;
}
