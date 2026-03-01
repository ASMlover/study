import { getTheme } from "./theme.js";
import type { AgentStage } from "../types.js";

// Smooth braille orbit animation
const FRAMES = ["⣷", "⣯", "⣟", "⡿", "⢿", "⣻", "⣽", "⣾"];

const STAGE_CONFIG: Record<AgentStage, { icon: string; label: string }> = {
  planning:      { icon: "◆", label: "Thinking" },
  coding:        { icon: "▸", label: "Working" },
  reviewing:     { icon: "◇", label: "Reviewing" },
  orchestrating: { icon: "●", label: "Responding" },
};

export class Spinner {
  private interval: ReturnType<typeof setInterval> | null = null;
  private frameIdx = 0;
  private stage: AgentStage = "planning";
  private detail = "";
  private startTime = 0;

  start(stage?: AgentStage): void {
    if (stage) this.stage = stage;
    this.frameIdx = 0;
    this.startTime = Date.now();
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
    const pulse = t.spinnerPulse;
    const colorIdx = this.frameIdx % pulse.length;
    const frame = FRAMES[this.frameIdx % FRAMES.length];
    const cfg = STAGE_CONFIG[this.stage];

    // Elapsed time
    const elapsed = ((Date.now() - this.startTime) / 1000).toFixed(0);
    const timeStr = `${t.dim}${elapsed}s${t.reset}`;

    // Detail text
    const detailStr = this.detail ? `${t.dim} · ${this.detail}${t.reset}` : "";

    process.stderr.write(
      `\r\x1b[K  ${pulse[colorIdx]}${frame}${t.reset} ${cfg.label}${detailStr} ${timeStr}`
    );
  }
}

export const spinner = new Spinner();
