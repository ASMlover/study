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
    const decoratedUrl = style(` (${url})`, ansi.gray, colorize);
    return `${decoratedLabel}${decoratedUrl}`;
  });
  out = out.replace(/\*\*([^*]+)\*\*/g, (_m, text: string) => style(text, ansi.bold, colorize));
  out = out.replace(/\*([^*]+)\*/g, (_m, text: string) => style(text, ansi.italic, colorize));
  out = out.replace(/^#{1,6}\s+/, "");
  out = out.replace(/\u0000CODE(\d+)\u0000/g, (_m, idx: string) => codeTokens[Number(idx)] ?? "");
  return out;
}

function renderLine(line: string, state: { inFence: boolean }, colorize: boolean): string {
  if (/^\s*```/.test(line)) {
    state.inFence = !state.inFence;
    return style(state.inFence ? "┌ code" : "└ end code", ansi.magenta, colorize);
  }

  if (state.inFence) {
    return style(line, ansi.code, colorize);
  }

  if (/^#{1,6}\s+/.test(line)) {
    return style(renderInline(line, colorize), ansi.bold, colorize);
  }
  if (/^>\s?/.test(line)) {
    return style(`| ${renderInline(line.replace(/^>\s?/, ""), colorize)}`, ansi.gray, colorize);
  }
  if (/^\s*([-*+]|\d+\.)\s+/.test(line)) {
    const content = line.replace(/^\s*([-*+]|\d+\.)\s+/, "");
    return `${style("•", ansi.blue, colorize)} ${renderInline(content, colorize)}`;
  }
  return renderInline(line, colorize);
}

export class MarkdownAnsiStreamRenderer {
  private remainder = "";
  private readonly state = { inFence: false };
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
