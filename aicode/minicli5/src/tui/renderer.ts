import { getTheme, BOX } from "./theme.js";

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
          // Close code block with bottom border
          this.state.inCodeBlock = false;
          output += `${t.codeBorder}  ${BOX.rbl}${BOX.h.repeat(50)}${BOX.rbr}${t.reset}\n`;
        } else {
          // Open code block with top border + language pill
          this.state.inCodeBlock = true;
          this.state.codeLang = line.slice(3).trim();
          this.state.codeLineNum = 0;
          const langPill = this.state.codeLang
            ? ` ${t.codeLang}${this.state.codeLang}${t.reset} `
            : "";
          output += `${t.codeBorder}  ${BOX.rtl}${BOX.h.repeat(3)}${t.reset}${langPill}${t.codeBorder}${BOX.h.repeat(Math.max(0, 46 - (this.state.codeLang?.length || 0)))}${BOX.rtr}${t.reset}\n`;
        }
        continue;
      }

      if (this.state.inCodeBlock) {
        this.state.codeLineNum++;
        const lineNo = String(this.state.codeLineNum).padStart(3);
        output += `${t.codeBorder}  ${BOX.v}${t.codeLineNo}${lineNo}${t.reset} ${t.codeBlock}${line}${t.reset}\n`;
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
      return `${t.codeBorder}  ${BOX.v}${t.codeLineNo}${lineNo}${t.reset} ${t.codeBlock}${line}${t.reset}`;
    }
    return this.renderLine(line);
  }

  private renderLine(line: string): string {
    const t = getTheme();

    // Headings — with decorative prefix
    const headingMatch = line.match(/^(#{1,6})\s+(.+)/);
    if (headingMatch) {
      const level = headingMatch[1].length;
      if (level <= 2) {
        return `\n  ${t.heading}${headingMatch[2]}${t.reset}`;
      }
      return `  ${t.subheading}${headingMatch[2]}${t.reset}`;
    }

    // Bullet points — indented with custom bullet
    if (line.match(/^\s*[-*]\s/)) {
      const indent = line.match(/^(\s*)/)?.[1] || "";
      const content = line.replace(/^\s*[-*]\s/, "");
      return `  ${indent}${t.accent}▸${t.reset} ${this.renderInline(content)}`;
    }

    // Numbered lists
    const numMatch = line.match(/^\s*(\d+)\.\s/);
    if (numMatch) {
      return `  ${t.muted}${numMatch[1]}.${t.reset} ${this.renderInline(line.replace(/^\s*\d+\.\s/, ""))}`;
    }

    // Blockquotes — with gradient bar
    if (line.startsWith("> ")) {
      return `  ${t.borderAccent}▌${t.reset} ${t.italic}${this.renderInline(line.slice(2))}${t.reset}`;
    }

    // Horizontal rules
    if (/^[-*_]{3,}\s*$/.test(line)) {
      return `  ${t.borderDim}${BOX.h.repeat(40)}${t.reset}`;
    }

    return `  ${this.renderInline(line)}`;
  }

  private renderInline(text: string): string {
    const t = getTheme();
    return text
      .replace(/`([^`]+)`/g, `${t.code}\`$1\`${t.reset}`)
      .replace(/\*\*([^*]+)\*\*/g, `${t.bold}$1${t.reset}`)
      .replace(/\*([^*]+)\*/g, `${t.italic}$1${t.reset}`);
  }
}

// ─── Tool Call Display ───────────────────────────────────────────────────────

export function renderToolCall(name: string, args: Record<string, unknown>): string {
  const t = getTheme();

  // Format args as key=value pairs
  const argsStr = Object.entries(args)
    .map(([k, v]) => {
      const val = typeof v === "string" ? v : JSON.stringify(v);
      const display = val.length > 60 ? val.slice(0, 57) + "..." : val;
      return `${t.toolArg}${k}${t.reset}${t.dim}=${t.reset}${t.muted}${display}${t.reset}`;
    })
    .join(" ");

  return `  ${t.toolIcon}⚡${t.reset} ${t.toolName}${name}${t.reset} ${argsStr}`;
}

export function renderToolResult(result: { success: boolean; output: string }): string {
  const t = getTheme();

  if (result.success) {
    const preview = result.output.length > 200 ? result.output.slice(0, 197) + "..." : result.output;
    return `     ${t.success}✓${t.reset} ${t.dim}${preview}${t.reset}`;
  }

  const preview = result.output.length > 200 ? result.output.slice(0, 197) + "..." : result.output;
  return `     ${t.error}✗${t.reset} ${t.dim}${preview}${t.reset}`;
}

export const renderer = new MarkdownRenderer();
