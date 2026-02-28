import * as readline from "node:readline";
import { getTheme } from "./theme.js";

export type ConfirmResult = "yes" | "always" | "no";

// Session-level allow-list: tools the user chose "always allow" for
const alwaysAllowed = new Set<string>();

export function isAlwaysAllowed(toolName: string): boolean {
  return alwaysAllowed.has(toolName);
}

export function clearAlwaysAllowed(): void {
  alwaysAllowed.clear();
}

// ─── Permission Box Rendering ─────────────────────────────────────────────────

function renderPermissionBox(toolName: string, detail: string, context?: string): string {
  const t = getTheme();
  const width = Math.min(process.stderr.columns || 80, 80);
  const innerW = width - 4; // │ + space + content + space + │
  const HR = "─".repeat(width - 2);

  const pad = (s: string, w: number) => {
    // Strip ANSI for length calc
    const visible = s.replace(/\x1b\[[0-9;]*m/g, "");
    const diff = w - visible.length;
    return diff > 0 ? s + " ".repeat(diff) : s;
  };

  const wrapLines = (text: string, maxW: number): string[] => {
    const result: string[] = [];
    for (const raw of text.split("\n")) {
      if (raw.length <= maxW) {
        result.push(raw);
      } else {
        let pos = 0;
        while (pos < raw.length) {
          result.push(raw.slice(pos, pos + maxW));
          pos += maxW;
        }
      }
    }
    return result;
  };

  const lines: string[] = [];

  // Top border
  lines.push(`${t.warning}┌${HR}┐${t.reset}`);

  // Title line
  const titleIcon = toolName === "shell" ? "⚙" : "✏";
  const titleLabel = toolName === "shell" ? "Shell Command" : toolName === "write_file" ? "Write File" : toolName;
  const title = `${titleIcon}  ${t.bold}${titleLabel}${t.reset}`;
  lines.push(`${t.warning}│${t.reset} ${pad(title, innerW)} ${t.warning}│${t.reset}`);

  // Separator
  lines.push(`${t.warning}├${HR}┤${t.reset}`);

  // Detail lines
  const detailLines = wrapLines(detail, innerW);
  for (const dl of detailLines) {
    lines.push(`${t.warning}│${t.reset} ${pad(dl, innerW)} ${t.warning}│${t.reset}`);
  }

  // Context (e.g., file content preview for write, command for shell)
  if (context) {
    lines.push(`${t.warning}│${t.reset} ${pad("", innerW)} ${t.warning}│${t.reset}`);
    const ctxLines = wrapLines(context, innerW).slice(0, 8); // Max 8 preview lines
    for (const cl of ctxLines) {
      lines.push(`${t.warning}│${t.reset} ${pad(`${t.dim}${cl}${t.reset}`, innerW)} ${t.warning}│${t.reset}`);
    }
    if (wrapLines(context, innerW).length > 8) {
      lines.push(`${t.warning}│${t.reset} ${pad(`${t.dim}... (truncated)${t.reset}`, innerW)} ${t.warning}│${t.reset}`);
    }
  }

  // Separator
  lines.push(`${t.warning}├${HR}┤${t.reset}`);

  // Options
  const opts = `${t.success}y${t.reset}=allow  ${t.primary}a${t.reset}=always  ${t.error}n${t.reset}=deny`;
  lines.push(`${t.warning}│${t.reset} ${pad(opts, innerW)} ${t.warning}│${t.reset}`);

  // Bottom border
  lines.push(`${t.warning}└${HR}┘${t.reset}`);

  return lines.join("\n");
}

// ─── Confirm Prompt ───────────────────────────────────────────────────────────

export async function confirmPermission(
  toolName: string,
  detail: string,
  context?: string,
): Promise<boolean> {
  // If user already chose "always allow" for this tool, skip prompt
  if (alwaysAllowed.has(toolName)) {
    const t = getTheme();
    process.stderr.write(`${t.dim}  ✓ auto-approved (${toolName})${t.reset}\n`);
    return true;
  }

  // Render permission box
  process.stderr.write("\n" + renderPermissionBox(toolName, detail, context) + "\n");

  // Ask with readline
  const answer = await askConfirm();

  if (answer === "always") {
    alwaysAllowed.add(toolName);
    const t = getTheme();
    process.stderr.write(`${t.success}  ✓ Allowed (always for this session)${t.reset}\n`);
    return true;
  }

  if (answer === "yes") {
    const t = getTheme();
    process.stderr.write(`${t.success}  ✓ Allowed${t.reset}\n`);
    return true;
  }

  const t = getTheme();
  process.stderr.write(`${t.error}  ✗ Denied${t.reset}\n`);
  return false;
}

function askConfirm(): Promise<ConfirmResult> {
  return new Promise((resolve) => {
    let resolved = false;
    const done = (v: ConfirmResult) => {
      if (resolved) return;
      resolved = true;
      resolve(v);
    };

    const t = getTheme();
    const rl = readline.createInterface({
      input: process.stdin,
      output: process.stderr,
      prompt: `${t.warning}?${t.reset} Allow? [y/a/n] `,
      terminal: true,
    });

    rl.on("line", (line) => {
      const ch = line.trim().toLowerCase();
      if (ch === "a" || ch === "always") {
        done("always");
      } else if (ch === "y" || ch === "yes" || ch === "") {
        done("yes");
      } else {
        done("no");
      }
      rl.close();
    });

    rl.on("close", () => {
      done("no");
    });

    rl.on("SIGINT", () => {
      done("no");
      rl.close();
    });

    rl.prompt();
  });
}
