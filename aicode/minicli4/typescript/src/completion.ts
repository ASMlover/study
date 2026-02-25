import fs from "node:fs";
import path from "node:path";

export interface CompletionContext {
  commandNames: string[];
  sessionIds: string[];
  configKeys: string[];
  toolNames: string[];
  projectRoot: string;
}

const SUBS: Record<string, string[]> = {
  "/config": ["get", "set", "list", "reset"],
  "/session": ["list", "new", "switch", "delete", "rename", "current"],
  "/tools": ["enable", "disable"],
  "/agents": ["list", "set"]
};

export function complete(text: string, ctx: CompletionContext): string[] {
  if (!text.startsWith("/")) {
    return [];
  }
  const trailing = text.endsWith(" ");
  const parts = text.trim().length > 0 ? text.trim().split(/\s+/) : [];
  if (parts.length <= 1 && !trailing) {
    const needle = parts[0] ?? "/";
    return ctx.commandNames.filter((name) => name.startsWith(needle)).sort();
  }

  const cmd = parts[0] ?? "";
  const subs = SUBS[cmd];
  if (subs) {
    if (parts.length === 1 && trailing) {
      return [...subs];
    }
    if (parts.length === 2 && !trailing) {
      return subs.filter((s) => s.startsWith(parts[1] ?? "")).sort();
    }
  }

  if (cmd === "/session" && parts[1] === "switch") {
    const needle = trailing ? "" : (parts[2] ?? "");
    return ctx.sessionIds.filter((x) => x.startsWith(needle)).sort();
  }

  if (cmd === "/config" && (parts[1] === "set" || parts[1] === "get")) {
    const needle = trailing ? "" : (parts[2] ?? "");
    return ctx.configKeys.filter((x) => x.startsWith(needle)).sort();
  }

  if (cmd === "/tools" && (parts[1] === "enable" || parts[1] === "disable")) {
    const needle = trailing ? "" : (parts[2] ?? "");
    return ctx.toolNames.filter((x) => x.startsWith(needle)).sort();
  }

  if (["/read", "/write", "/ls", "/grep", "/tree", "/export", "/add"].includes(cmd)) {
    const needle = trailing ? "" : (parts[parts.length - 1] ?? "");
    return completePaths(ctx.projectRoot, needle);
  }

  return [];
}

function completePaths(projectRoot: string, prefix: string): string[] {
  const all: string[] = [];
  const walk = (dir: string): void => {
    const entries = fs.readdirSync(dir, { withFileTypes: true }).slice(0, 120);
    for (const entry of entries) {
      if (entry.name === ".git" || entry.name === "node_modules" || entry.name === ".minicli4") {
        continue;
      }
      const full = path.join(dir, entry.name);
      const rel = path.relative(projectRoot, full).replace(/\\/g, "/");
      all.push(rel);
      if (entry.isDirectory() && rel.split("/").length < 6) {
        walk(full);
      }
      if (all.length > 250) {
        return;
      }
    }
  };
  try {
    walk(projectRoot);
  } catch {
    return [];
  }
  return all.filter((x) => x.startsWith(prefix)).sort();
}
