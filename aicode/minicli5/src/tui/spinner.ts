import { getTheme } from "./theme.js";
import type { AgentStage } from "../types.js";

const FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];

const STAGE_LABELS: Record<AgentStage, string> = {
  planning: "Thinking",
  coding: "Working",
  reviewing: "Reviewing",
  orchestrating: "Responding",
};

export class Spinner {
  private interval: ReturnType<typeof setInterval> | null = null;
  private frameIdx = 0;
  private stage: AgentStage = "planning";
  private detail = "";

  start(stage?: AgentStage): void {
    if (stage) this.stage = stage;
    this.frameIdx = 0;
    this.stop();

    this.interval = setInterval(() => {
      this.render();
      this.frameIdx = (this.frameIdx + 1) % FRAMES.length;
    }, 80);
  }

  setStage(stage: AgentStage, detail?: string): void {
    this.stage = stage;
    this.detail = detail ?? "";
  }

  stop(): void {
    if (this.interval) {
      clearInterval(this.interval);
      this.interval = null;
      process.stderr.write("\r\x1b[K");
    }
  }

  private render(): void {
    const t = getTheme();
    const frame = FRAMES[this.frameIdx];
    const label = STAGE_LABELS[this.stage];
    const detailStr = this.detail ? ` ${t.dim}${this.detail}${t.reset}` : "";
    process.stderr.write(`\r\x1b[K${t.spinner}${frame}${t.reset} ${label}${detailStr}`);
  }
}

export const spinner = new Spinner();
