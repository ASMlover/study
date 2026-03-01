import readline from "node:readline";

const SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];
const RESET = "\x1b[0m";
const COLORS = {
  cyan: "\x1b[36m",
  blue: "\x1b[34m",
  green: "\x1b[32m",
  yellow: "\x1b[33m",
  red: "\x1b[31m",
  magenta: "\x1b[35m",
  gray: "\x1b[90m",
  white: "\x1b[97m",
  bold: "\x1b[1m",
};
const SPINNER_COLOR_CYCLE = [COLORS.cyan, COLORS.blue, COLORS.magenta, COLORS.green];

export class UI {
  constructor() {
    this.rl = readline.createInterface({
      input: process.stdin,
      output: process.stdout,
      terminal: true,
    });
    this.spinnerTimer = null;
    this.spinnerIndex = 0;
    this.spinnerText = "";
    this.streaming = false;
    this.useColor = Boolean(process.stdout.isTTY);
  }

  printBanner(config) {
    console.log(`\n${this.c("minicli7 - AI Agent CLI", `${COLORS.bold}${COLORS.cyan}`)}`);
    console.log(
      `${this.c("model", COLORS.green)}: ${this.c(config.model, COLORS.white)} | ${this.c("endpoint", COLORS.green)}: ${this.c(config.apiBaseUrl, COLORS.gray)}`
    );
    if (Array.isArray(config.warnings) && config.warnings.length) {
      for (const warning of config.warnings) {
        console.log(`${this.c("config warning", COLORS.yellow)}: ${this.c(warning, COLORS.yellow)}`);
      }
    }
    console.log(
      `${this.c("commands", COLORS.blue)}: ${this.c("/help /status /context /todo /task /bg /skills /agent /compact /exit", COLORS.gray)}\n`
    );
  }

  printHelp() {
    console.log(this.c("Commands:", `${COLORS.bold}${COLORS.cyan}`));
    console.log("  /help");
    console.log("  /status");
    console.log("  /context");
    console.log("  /exit");
    console.log("  /todo add <text>");
    console.log("  /todo done <id>");
    console.log("  /todo list");
    console.log("  /task add <title> [--deps=1,2] [--goal=text]");
    console.log("  /task list");
    console.log("  /task run");
    console.log("  /bg run <shell-command>");
    console.log("  /bg list");
    console.log("  /skills list");
    console.log("  /skills show <name>");
    console.log("  /skills run <name> <input>");
    console.log("  /agent <goal>");
    console.log("  /compact");
    console.log("  /clear");
  }

  prompt(promptText = "you> ") {
    const styledPrompt =
      promptText === "you> "
        ? `${this.c("you", `${COLORS.bold}${COLORS.green}`)}${this.c(">", COLORS.green)} `
        : promptText;

    return new Promise((resolve) => {
      this.rl.question(styledPrompt, (answer) => resolve(answer));
    });
  }

  async confirm(question) {
    const q = `${this.c(question, COLORS.white)} ${this.c("[y/N]", COLORS.yellow)} `;
    const answer = (await this.prompt(q)).trim().toLowerCase();
    return answer === "y" || answer === "yes";
  }

  notify(message) {
    console.log(this.c(message, COLORS.gray));
  }

  error(message) {
    console.error(`${this.c("Error", `${COLORS.bold}${COLORS.red}`)}: ${this.c(message, COLORS.red)}`);
  }

  startSpinner(text = "assistant thinking") {
    if (this.spinnerTimer) {
      return;
    }
    this.spinnerText = text;
    this.spinnerIndex = 0;
    this.spinnerTimer = setInterval(() => {
      const frame = SPINNER_FRAMES[this.spinnerIndex % SPINNER_FRAMES.length];
      const color = SPINNER_COLOR_CYCLE[this.spinnerIndex % SPINNER_COLOR_CYCLE.length];
      const dots = ".".repeat((this.spinnerIndex % 3) + 1);
      const pad = " ".repeat(3 - dots.length);
      const line = `${this.c(frame, `${COLORS.bold}${color}`)} ${this.c(this.spinnerText, COLORS.white)}${this.c(
        dots,
        color
      )}${pad}`;
      process.stdout.write(`\r${line}`);
      this.spinnerIndex += 1;
    }, 100);
  }

  stopSpinner() {
    if (!this.spinnerTimer) {
      return;
    }
    clearInterval(this.spinnerTimer);
    this.spinnerTimer = null;
    process.stdout.write("\r\x1b[2K");
  }

  beginAssistantStream() {
    this.streaming = true;
    process.stdout.write(`${this.c("assistant", `${COLORS.bold}${COLORS.blue}`)}${this.c(">", COLORS.blue)} `);
  }

  pushAssistantToken(token) {
    if (!this.streaming) {
      this.beginAssistantStream();
    }
    process.stdout.write(this.c(token, COLORS.white));
  }

  endAssistantStream() {
    if (this.streaming) {
      process.stdout.write("\n");
    }
    this.streaming = false;
  }

  close() {
    this.stopSpinner();
    this.rl.close();
  }

  c(text, color) {
    if (!this.useColor) {
      return text;
    }
    return `${color}${text}${RESET}`;
  }
}
