import * as readline from "node:readline";
import { getTheme } from "./theme.js";

export interface PromptOptions {
  skills: string[];
  onAbort: () => void;
}

export class Prompt {
  private rl: readline.Interface | null = null;
  private skills: string[] = [];
  private onAbort: () => void = () => {};
  private history: string[] = [];

  init(opts: PromptOptions): void {
    this.skills = opts.skills;
    this.onAbort = opts.onAbort;
  }

  async readInput(): Promise<string | null> {
    return new Promise((resolve) => {
      let resolved = false;
      const done = (value: string | null) => {
        if (resolved) return;
        resolved = true;
        resolve(value);
      };

      const t = getTheme();
      // Distinctive two-char prompt: dim triangle + bright chevron
      const promptStr = `${t.dim}▲${t.reset} ${t.promptChar}›${t.reset} `;

      this.rl = readline.createInterface({
        input: process.stdin,
        output: process.stderr,
        prompt: promptStr,
        terminal: true,
        completer: (line: string) => this.complete(line),
        history: this.history,
      });

      this.rl.on("line", (line) => {
        const trimmed = line.trim();
        if (trimmed) {
          this.history.push(trimmed);
        }
        done(trimmed || "");
        this.rl?.close();
        this.rl = null;
      });

      this.rl.on("close", () => {
        this.rl = null;
        done(null);
      });

      this.rl.on("SIGINT", () => {
        this.onAbort();
        done(null);
        this.rl?.close();
        this.rl = null;
      });

      this.rl.prompt();
    });
  }

  close(): void {
    this.rl?.close();
    this.rl = null;
  }

  private complete(line: string): [string[], string] {
    if (line.startsWith("/")) {
      const prefix = line.slice(1);
      const matches = this.skills
        .filter(s => s.startsWith(prefix))
        .map(s => `/${s}`);
      return [matches.length ? matches : this.skills.map(s => `/${s}`), line];
    }
    return [[], line];
  }
}

export const prompt = new Prompt();
