import { ansi } from "./ansi";

export interface MarkdownRenderOptions {
  colorize?: boolean;
}

function style(text: string, fn: (value: string) => string, colorize: boolean): string {
  return colorize ? fn(text) : text;
}

function renderInline(line: string, colorize: boolean): string {
  const codeTokens: string[] = [];
  let out = line.replace(/`([^`]+)`/g, (_m, code: string) => {
    const token = `\u0000CODE${codeTokens.length}\u0000`;
    codeTokens.push(style(code, ansi.code, colorize));
    return token;
  });

  out = out.replace(/\[([^\]]+)\]\(([^)]+)\)/g, (_m, label: string, url: string) => {
    const decoratedLabel = style(label, ansi.cyan, colorize);
    const decoratedUrl = style(` <${url}>`, ansi.gray, colorize);
    return `${decoratedLabel}${decoratedUrl}`;
  });

  out = out.replace(/\*\*([^*]+)\*\*/g, (_m, text: string) => style(text, ansi.bold, colorize));
  out = out.replace(/\*([^*]+)\*/g, (_m, text: string) => style(text, ansi.italic, colorize));
  out = out.replace(/^#{1,6}\s+/, "");
  out = out.replace(/\u0000CODE(\d+)\u0000/g, (_m, idx: string) => codeTokens[Number(idx)] ?? "");
  return out;
}

function renderLine(
  line: string,
  state: { inFence: boolean; fenceLang: string; codeLine: number },
  colorize: boolean
): string {
  const fenceMatch = line.match(/^\s*```\s*([a-zA-Z0-9_-]+)?\s*$/);
  if (fenceMatch) {
    if (!state.inFence) {
      state.inFence = true;
      state.fenceLang = fenceMatch[1] ?? "text";
      state.codeLine = 1;
      return style(`[code:${state.fenceLang}]`, ansi.magenta, colorize);
    }
    state.inFence = false;
    state.fenceLang = "";
    state.codeLine = 1;
    return style("[/code]", ansi.magenta, colorize);
  }

  if (state.inFence) {
    const numbered = `${String(state.codeLine).padStart(3, " ")} | ${line}`;
    state.codeLine += 1;
    return style(numbered, ansi.code, colorize);
  }

  const heading = line.match(/^(#{1,6})\s+(.+)$/);
  if (heading) {
    const level = heading[1].length;
    const marker = `${"#".repeat(level)}`;
    return `${style(marker, ansi.yellow, colorize)} ${style(renderInline(heading[2], colorize), ansi.bold, colorize)}`;
  }

  if (/^>\s?/.test(line)) {
    return `${style("|", ansi.gray, colorize)} ${renderInline(line.replace(/^>\s?/, ""), colorize)}`;
  }

  const ordered = line.match(/^\s*(\d+)\.\s+(.+)$/);
  if (ordered) {
    return `${style(`${ordered[1]}.`, ansi.blue, colorize)} ${renderInline(ordered[2], colorize)}`;
  }

  const unordered = line.match(/^\s*[-*+]\s+(.+)$/);
  if (unordered) {
    return `${style("-", ansi.blue, colorize)} ${renderInline(unordered[1], colorize)}`;
  }

  const tableRow = line.match(/^\s*\|(.+)\|\s*$/);
  if (tableRow) {
    const raw = tableRow[1].split("|").map((cell) => cell.trim());
    const isDivider = raw.every((cell) => /^:?-{3,}:?$/.test(cell));
    if (isDivider) {
      return style(`| ${raw.map((cell) => cell.replace(/:/g, "-")).join(" | ")} |`, ansi.gray, colorize);
    }
    const rendered = raw.map((cell) => renderInline(cell, colorize));
    return `${style("|", ansi.gray, colorize)} ${rendered.join(` ${style("|", ansi.gray, colorize)} `)} ${style("|", ansi.gray, colorize)}`;
  }

  return renderInline(line, colorize);
}

export class MarkdownAnsiStreamRenderer {
  private remainder = "";
  private readonly state = { inFence: false, fenceLang: "", codeLine: 1 };

  constructor(private readonly options: MarkdownRenderOptions = {}) {}

  write(chunk: string): string {
    this.remainder += chunk;
    let output = "";
    let newlineAt = this.remainder.indexOf("\n");
    while (newlineAt >= 0) {
      const line = this.remainder.slice(0, newlineAt);
      this.remainder = this.remainder.slice(newlineAt + 1);
      output += `${renderLine(line, this.state, Boolean(this.options.colorize))}\n`;
      newlineAt = this.remainder.indexOf("\n");
    }
    return output;
  }

  flush(): string {
    if (!this.remainder) {
      return "";
    }
    const line = this.remainder;
    this.remainder = "";
    return renderLine(line, this.state, Boolean(this.options.colorize));
  }
}
