import { getTheme } from "./theme.js";

interface RenderState {
  inCodeBlock: boolean;
  codeLang: string;
  codeLineNum: number;
  buffer: string;
}

export class MarkdownRenderer {
  private state: RenderState = {
    inCodeBlock: false,
    codeLang: "",
    codeLineNum: 0,
    buffer: "",
  };

  reset(): void {
    this.state = { inCodeBlock: false, codeLang: "", codeLineNum: 0, buffer: "" };
  }

  renderChunk(text: string): string {
    this.state.buffer += text;
    let output = "";
    const t = getTheme();

    while (this.state.buffer.includes("\n")) {
      const idx = this.state.buffer.indexOf("\n");
      const line = this.state.buffer.slice(0, idx);
      this.state.buffer = this.state.buffer.slice(idx + 1);

      if (line.startsWith("```")) {
        if (this.state.inCodeBlock) {
          this.state.inCodeBlock = false;
          output += `${t.reset}\n`;
        } else {
          this.state.inCodeBlock = true;
          this.state.codeLang = line.slice(3).trim();
          this.state.codeLineNum = 0;
          const langLabel = this.state.codeLang ? ` ${t.dim}${this.state.codeLang}${t.reset}` : "";
          output += `${t.secondary}┌──${langLabel}\n`;
        }
        continue;
      }

      if (this.state.inCodeBlock) {
        this.state.codeLineNum++;
        const lineNo = String(this.state.codeLineNum).padStart(3);
        output += `${t.secondary}│${t.dim}${lineNo}${t.reset} ${t.codeBlock}${line}${t.reset}\n`;
        continue;
      }

      output += this.renderLine(line) + "\n";
    }

    return output;
  }

  flush(): string {
    if (!this.state.buffer) return "";
    const line = this.state.buffer;
    this.state.buffer = "";

    if (this.state.inCodeBlock) {
      const t = getTheme();
      this.state.codeLineNum++;
      const lineNo = String(this.state.codeLineNum).padStart(3);
      return `${t.secondary}│${t.dim}${lineNo}${t.reset} ${t.codeBlock}${line}${t.reset}`;
    }
    return this.renderLine(line);
  }

  private renderLine(line: string): string {
    const t = getTheme();

    // Headings
    const headingMatch = line.match(/^(#{1,6})\s+(.+)/);
    if (headingMatch) {
      return `${t.heading}${headingMatch[1]} ${headingMatch[2]}${t.reset}`;
    }

    // Bullet points
    if (line.match(/^\s*[-*]\s/)) {
      return `${t.secondary}•${t.reset} ${this.renderInline(line.replace(/^\s*[-*]\s/, ""))}`;
    }

    // Numbered lists
    const numMatch = line.match(/^\s*(\d+)\.\s/);
    if (numMatch) {
      return `${t.secondary}${numMatch[1]}.${t.reset} ${this.renderInline(line.replace(/^\s*\d+\.\s/, ""))}`;
    }

    // Blockquotes
    if (line.startsWith("> ")) {
      return `${t.secondary}│${t.reset} ${t.dim}${this.renderInline(line.slice(2))}${t.reset}`;
    }

    // Horizontal rules
    if (/^[-*_]{3,}\s*$/.test(line)) {
      return `${t.secondary}${"─".repeat(40)}${t.reset}`;
    }

    return this.renderInline(line);
  }

  private renderInline(text: string): string {
    const t = getTheme();
    return text
      .replace(/`([^`]+)`/g, `${t.code}\`$1\`${t.reset}`)
      .replace(/\*\*([^*]+)\*\*/g, `${t.bold}$1${t.reset}`)
      .replace(/\*([^*]+)\*/g, `${t.dim}$1${t.reset}`);
  }
}

export function renderToolCall(name: string, args: Record<string, unknown>): string {
  const t = getTheme();
  const argsStr = Object.entries(args)
    .map(([k, v]) => {
      const val = typeof v === "string" ? v : JSON.stringify(v);
      const display = val.length > 60 ? val.slice(0, 57) + "..." : val;
      return `${t.toolArg}${k}${t.reset}=${t.dim}${display}${t.reset}`;
    })
    .join(" ");
  return `${t.secondary}⚡${t.reset} ${t.toolName}${name}${t.reset} ${argsStr}`;
}

export function renderToolResult(result: { success: boolean; output: string }): string {
  const t = getTheme();
  const icon = result.success ? `${t.success}✓${t.reset}` : `${t.error}✗${t.reset}`;
  const preview = result.output.length > 200 ? result.output.slice(0, 197) + "..." : result.output;
  return `  ${icon} ${t.dim}${preview}${t.reset}`;
}

export const renderer = new MarkdownRenderer();
