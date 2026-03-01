import * as readline from "node:readline";
import { getTheme, BOX } from "./theme.js";

export type ConfirmResult = "yes" | "always" | "no";

// Session-level allow-list: tools the user chose "always allow" for
const alwaysAllowed = new Set<string>();

export function isAlwaysAllowed(toolName: string): boolean {
  return alwaysAllowed.has(toolName);
}

export function clearAlwaysAllowed(): void {
  alwaysAllowed.clear();
}

// ─── Tool Icons ──────────────────────────────────────────────────────────────

const TOOL_ICONS: Record<string, string> = {
  shell:       "⚙",
  write_file:  "✎",
  read_file:   "◉",
  list_dir:    "☰",
  grep:        "⌕",
  glob:        "✦",
  default:     "◈",
};

function getToolIcon(toolName: string): string {
  return TOOL_ICONS[toolName] ?? TOOL_ICONS.default;
}

function getToolLabel(toolName: string): string {
  const labels: Record<string, string> = {
    shell: "Shell Command",
    write_file: "Write File",
    read_file: "Read File",
    list_dir: "List Directory",
    grep: "Search Content",
    glob: "Find Files",
  };
  return labels[toolName] ?? toolName;
}

// ─── Permission Box ─────────────────────────────────────────────────────────

function renderPermissionBox(toolName: string, detail: string, context?: string): string {
  const t = getTheme();
  const width = Math.min(process.stderr.columns || 80, 76);
  const innerW = width - 4;
  const HR = BOX.h.repeat(width - 2);

  const pad = (s: string, w: number) => {
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
  const icon = getToolIcon(toolName);
  const label = getToolLabel(toolName);

  // Top border — rounded with accent color
  lines.push(`  ${t.borderAccent}${BOX.rtl}${HR}${BOX.rtr}${t.reset}`);

  // Title line with icon
  const title = `${t.warning}${icon}${t.reset}  ${t.bold}${label}${t.reset}`;
  lines.push(`  ${t.borderAccent}${BOX.v}${t.reset} ${pad(title, innerW)} ${t.borderAccent}${BOX.v}${t.reset}`);

  // Separator
  lines.push(`  ${t.borderAccent}${BOX.lt}${HR}${BOX.rt}${t.reset}`);

  // Detail lines
  const detailLines = wrapLines(detail, innerW);
  for (const dl of detailLines) {
    lines.push(`  ${t.border}${BOX.v}${t.reset} ${pad(dl, innerW)} ${t.border}${BOX.v}${t.reset}`);
  }

  // Context preview (file content, command, etc.)
  if (context) {
    lines.push(`  ${t.border}${BOX.v}${t.reset} ${pad("", innerW)} ${t.border}${BOX.v}${t.reset}`);
    const ctxLines = wrapLines(context, innerW).slice(0, 8);
    for (const cl of ctxLines) {
      lines.push(`  ${t.border}${BOX.v}${t.reset} ${pad(`${t.dim}${cl}${t.reset}`, innerW)} ${t.border}${BOX.v}${t.reset}`);
    }
    if (wrapLines(context, innerW).length > 8) {
      lines.push(`  ${t.border}${BOX.v}${t.reset} ${pad(`${t.dim}... (truncated)${t.reset}`, innerW)} ${t.border}${BOX.v}${t.reset}`);
    }
  }

  // Option bar
  lines.push(`  ${t.border}${BOX.lt}${HR}${BOX.rt}${t.reset}`);
  const opts = `${t.success}y${t.reset} allow  ${t.primary}a${t.reset} always  ${t.error}n${t.reset} deny`;
  lines.push(`  ${t.border}${BOX.v}${t.reset} ${pad(opts, innerW)} ${t.border}${BOX.v}${t.reset}`);

  // Bottom border — rounded
  lines.push(`  ${t.borderAccent}${BOX.rbl}${HR}${BOX.rbr}${t.reset}`);

  return lines.join("\n");
}

// ─── Confirm Prompt ──────────────────────────────────────────────────────────

export async function confirmPermission(
  toolName: string,
  detail: string,
  context?: string,
): Promise<boolean> {
  // If user already chose "always allow" for this tool, skip prompt
  if (alwaysAllowed.has(toolName)) {
    const t = getTheme();
    process.stderr.write(`     ${t.dim}✓ auto-approved${t.reset} ${t.muted}(${toolName})${t.reset}\n`);
    return true;
  }

  // Render permission box
  process.stderr.write("\n" + renderPermissionBox(toolName, detail, context) + "\n");

  // Ask with readline
  const answer = await askConfirm();

  if (answer === "always") {
    alwaysAllowed.add(toolName);
    const t = getTheme();
    process.stderr.write(`  ${t.success}✓ Allowed${t.reset} ${t.dim}(always for this session)${t.reset}\n`);
    return true;
  }

  if (answer === "yes") {
    const t = getTheme();
    process.stderr.write(`  ${t.success}✓ Allowed${t.reset}\n`);
    return true;
  }

  const t = getTheme();
  process.stderr.write(`  ${t.error}✗ Denied${t.reset}\n`);
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
      prompt: `  ${t.warning}?${t.reset} Allow? ${t.dim}[y/a/n]${t.reset} `,
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
