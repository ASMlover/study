import readline from "node:readline";

const SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"];
const RESET = "\x1b[0m";
const COLORS = {
  cyan: "\x1b[38;5;81m",
  blue: "\x1b[38;5;75m",
  green: "\x1b[38;5;114m",
  yellow: "\x1b[38;5;220m",
  amber: "\x1b[38;5;214m",
  red: "\x1b[38;5;203m",
  magenta: "\x1b[38;5;177m",
  gray: "\x1b[90m",
  white: "\x1b[97m",
  dim: "\x1b[2m",
  bold: "\x1b[1m",
};
const BG = {
  cyanSoft: "\x1b[48;5;24m",
  amberSoft: "\x1b[48;5;58m",
  redSoft: "\x1b[48;5;52m",
  slateSoft: "\x1b[48;5;238m",
};
const SPINNER_COLOR_CYCLE = [COLORS.cyan, COLORS.blue, COLORS.magenta, COLORS.amber];

export class UI {
  constructor() {
    this.slashCommands = [];
    this.slashSubcommands = {};
    this.slashArgumentHints = {};

    this.rl = readline.createInterface({
      input: process.stdin,
      output: process.stdout,
      terminal: true,
      completer: (line) => this.completeInput(line),
    });
    this.spinnerTimer = null;
    this.spinnerIndex = 0;
    this.spinnerText = "";
    this.spinnerStartTs = 0;
    this.streaming = false;
    this.useColor = Boolean(process.stdout.isTTY);
    this.columns = process.stdout.columns || 100;

    process.stdout.on("resize", () => {
      this.columns = process.stdout.columns || 100;
    });
  }

  setSlashCompletionSpec(spec = {}) {
    this.slashCommands = Array.isArray(spec.commands) ? [...spec.commands] : [];
    this.slashSubcommands = spec.subcommands && typeof spec.subcommands === "object" ? { ...spec.subcommands } : {};
    this.slashArgumentHints =
      spec.argumentHints && typeof spec.argumentHints === "object" ? { ...spec.argumentHints } : {};
  }

  printBanner(config) {
    const title = this.c(" MINICLI7 ", `${COLORS.bold}${COLORS.amber}${BG.slateSoft}`);
    const subtitle = this.c("Agent Terminal", `${COLORS.bold}${COLORS.cyan}`);
    const modelBadge = this.badge(`model ${config.model}`, "info");
    const endpoint = this.c(config.apiBaseUrl, `${COLORS.dim}${COLORS.gray}`);

    console.log("");
    console.log(`${title}  ${subtitle}`);
    this.printDivider("thin");
    console.log(`${modelBadge}  ${this.c("endpoint", COLORS.gray)} ${endpoint}`);

    if (Array.isArray(config.warnings) && config.warnings.length) {
      for (const warning of config.warnings) {
        console.log(`${this.badge("config warning", "warn")} ${this.c(warning, COLORS.yellow)}`);
      }
    }
    console.log(`${this.c("quick", COLORS.gray)} ${this.c("/help  /status  /todo list  /task list  /exit", COLORS.gray)}`);
    this.printDivider("thick");
    console.log("");
  }

  printHelp() {
    this.section("Core");
    this.printCommand("/help", "show command help");
    this.printCommand("/status | /context", "show context usage");
    this.printCommand("/compact", "compact conversation memory");
    this.printCommand("/clear", "clear conversation context");
    this.printCommand("/exit", "quit session");

    this.section("Todo");
    this.printCommand("/todo list", "show todos");
    this.printCommand("/todo add <text>", "create todo");
    this.printCommand("/todo done <id>", "mark todo done");

    this.section("Tasks");
    this.printCommand("/task list", "show task graph");
    this.printCommand("/task add <title> [--deps=1,2] [--goal=text]", "create task");
    this.printCommand("/task run", "run ready tasks");

    this.section("Tools");
    this.printCommand("/bg run <shell-command>", "start background command");
    this.printCommand("/bg list", "show background jobs");
    this.printCommand("/agent <goal>", "launch sub-agent run");

    this.section("Skills");
    this.printCommand("/skills list", "list local skills");
    this.printCommand("/skills show <name>", "inspect skill file");
    this.printCommand("/skills run <name> <input>", "run skill prompt");

    this.printDivider("thin");
  }

  prompt(promptText = "you> ") {
    const styledPrompt =
      promptText === "you> "
        ? `${this.c(this.timeTag(), COLORS.gray)} ${this.badge("you", "ok")} ${this.c("›", COLORS.green)} `
        : promptText;

    return new Promise((resolve) => {
      this.rl.question(styledPrompt, (answer) => resolve(answer));
    });
  }

  async confirm(question) {
    const q = `${this.badge("confirm", "warn")} ${this.c(question, COLORS.white)} ${this.c("[y/N]", COLORS.yellow)} `;
    const answer = (await this.prompt(q)).trim().toLowerCase();
    return answer === "y" || answer === "yes";
  }

  notify(message, tone = "info") {
    const colorMap = {
      info: COLORS.gray,
      ok: COLORS.green,
      warn: COLORS.yellow,
      tool: COLORS.cyan,
      task: COLORS.blue,
    };
    const markerMap = {
      info: "·",
      ok: "✓",
      warn: "!",
      tool: "⚙",
      task: "▣",
    };
    const color = colorMap[tone] || COLORS.gray;
    const marker = markerMap[tone] || "·";
    console.log(`${this.c(marker, `${COLORS.bold}${color}`)} ${this.c(message, color)}`);
  }

  error(message) {
    console.error(`${this.badge("error", "error")} ${this.c(message, COLORS.red)}`);
  }

  startSpinner(text = "assistant thinking") {
    if (this.spinnerTimer) {
      return;
    }
    this.spinnerText = text;
    this.spinnerIndex = 0;
    this.spinnerStartTs = Date.now();
    this.spinnerTimer = setInterval(() => {
      const frame = SPINNER_FRAMES[this.spinnerIndex % SPINNER_FRAMES.length];
      const color = SPINNER_COLOR_CYCLE[this.spinnerIndex % SPINNER_COLOR_CYCLE.length];
      const elapsed = ((Date.now() - this.spinnerStartTs) / 1000).toFixed(1);
      const pulse = "█".repeat((this.spinnerIndex % 4) + 1).padEnd(4, "·");
      const line = `${this.c(frame, `${COLORS.bold}${color}`)} ${this.c(this.spinnerText, COLORS.white)} ${this.c(
        pulse,
        color
      )} ${this.c(`${elapsed}s`, COLORS.gray)}`;
      process.stdout.write(`\r\x1b[2K${line}`);
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
    process.stdout.write(
      `${this.c(this.timeTag(), COLORS.gray)} ${this.badge("assistant", "assistant")} ${this.c("╭─ ", COLORS.blue)}`
    );
  }

  pushAssistantToken(token) {
    if (!this.streaming) {
      this.beginAssistantStream();
    }
    process.stdout.write(this.c(token, COLORS.white));
  }

  endAssistantStream() {
    if (this.streaming) {
      process.stdout.write(`\n${this.c(" ".repeat(13), COLORS.gray)}${this.c("╰─ done", COLORS.gray)}\n`);
    }
    this.streaming = false;
  }

  close() {
    this.stopSpinner();
    this.rl.close();
  }

  completeInput(line) {
    if (!line.startsWith("/")) {
      return [[], line];
    }

    const hasTrailingSpace = /\s$/.test(line);
    const parts = line.trim().split(/\s+/).filter(Boolean);
    if (!parts.length) {
      return [this.slashCommands, line];
    }

    const cmd = parts[0];
    if (parts.length === 1 && !hasTrailingSpace) {
      const matches = this.slashCommands.filter((candidate) => candidate.startsWith(cmd));
      return [matches.length ? matches : this.slashCommands, cmd];
    }

    const subcommands = this.slashSubcommands[cmd];
    if (!Array.isArray(subcommands) || !subcommands.length) {
      return [[], line];
    }

    const sub = hasTrailingSpace ? "" : parts[1] || "";
    if (parts.length <= 2) {
      const matches = subcommands.filter((candidate) => candidate.startsWith(sub)).map((candidate) => `${cmd} ${candidate}`);
      return [matches.length ? matches : subcommands.map((candidate) => `${cmd} ${candidate}`), line];
    }

    const argKey = `${cmd} ${parts[1]}`;
    const hints = this.slashArgumentHints[argKey];
    if (!Array.isArray(hints) || !hints.length) {
      return [[], line];
    }

    const currentToken = hasTrailingSpace ? "" : parts[parts.length - 1];
    const prefix = hasTrailingSpace ? line : line.slice(0, line.length - currentToken.length);
    const matches = hints.filter((candidate) => candidate.startsWith(currentToken)).map((candidate) => `${prefix}${candidate}`);
    return [matches, currentToken];
  }

  c(text, color) {
    if (!this.useColor) {
      return text;
    }
    return `${color}${text}${RESET}`;
  }

  badge(text, tone = "info") {
    const map = {
      info: { fg: COLORS.cyan, bg: BG.cyanSoft },
      ok: { fg: COLORS.green, bg: BG.slateSoft },
      warn: { fg: COLORS.yellow, bg: BG.amberSoft },
      error: { fg: COLORS.red, bg: BG.redSoft },
      assistant: { fg: COLORS.blue, bg: BG.slateSoft },
    };
    const t = map[tone] || map.info;
    return this.c(` ${text.toUpperCase()} `, `${COLORS.bold}${t.fg}${t.bg}`);
  }

  section(name) {
    console.log(`\n${this.badge(name, "info")}`);
  }

  printCommand(command, desc) {
    const left = this.c(command.padEnd(42, " "), COLORS.white);
    const right = this.c(desc, COLORS.gray);
    console.log(`  ${left} ${right}`);
  }

  printDivider(style = "thin") {
    const width = Math.max(36, Math.min(this.columns - 2, 110));
    const line = style === "thick" ? "═".repeat(width) : "─".repeat(width);
    const color = style === "thick" ? COLORS.gray : `${COLORS.dim}${COLORS.gray}`;
    console.log(this.c(line, color));
  }

  timeTag() {
    const now = new Date();
    const hh = String(now.getHours()).padStart(2, "0");
    const mm = String(now.getMinutes()).padStart(2, "0");
    const ss = String(now.getSeconds()).padStart(2, "0");
    return `${hh}:${mm}:${ss}`;
  }
}
