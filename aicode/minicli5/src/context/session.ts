import { readFileSync, writeFileSync, readdirSync, unlinkSync, existsSync, mkdirSync } from "node:fs";
import { join } from "node:path";
import type { Session, AgentTurn, Task } from "../types.js";
import { log } from "../utils/logger.js";

function sessionsDir(projectRoot: string): string {
  const dir = join(projectRoot, ".minicli5", "sessions");
  if (!existsSync(dir)) mkdirSync(dir, { recursive: true });
  return dir;
}

function sessionPath(projectRoot: string, id: string): string {
  return join(sessionsDir(projectRoot), `${id}.json`);
}

export function createSession(projectRoot: string, name?: string): Session {
  const id = Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
  const session: Session = {
    id,
    name: name ?? `session-${id}`,
    turns: [],
    tasks: [],
    createdAt: Date.now(),
    updatedAt: Date.now(),
  };
  saveSession(projectRoot, session);
  log("info", "Session created", { id });
  return session;
}

export function saveSession(projectRoot: string, session: Session): void {
  session.updatedAt = Date.now();
  const path = sessionPath(projectRoot, session.id);
  writeFileSync(path, JSON.stringify(session, null, 2), "utf-8");
}

export function loadSession(projectRoot: string, id: string): Session | null {
  const path = sessionPath(projectRoot, id);
  if (!existsSync(path)) return null;
  try {
    return JSON.parse(readFileSync(path, "utf-8")) as Session;
  } catch {
    return null;
  }
}

export function listSessions(projectRoot: string): Array<{ id: string; name: string; updatedAt: number }> {
  const dir = sessionsDir(projectRoot);
  try {
    return readdirSync(dir)
      .filter(f => f.endsWith(".json"))
      .map(f => {
        try {
          const data = JSON.parse(readFileSync(join(dir, f), "utf-8")) as Session;
          return { id: data.id, name: data.name, updatedAt: data.updatedAt };
        } catch {
          return null;
        }
      })
      .filter((s): s is NonNullable<typeof s> => s !== null)
      .sort((a, b) => b.updatedAt - a.updatedAt);
  } catch {
    return [];
  }
}

export function deleteSession(projectRoot: string, id: string): boolean {
  const path = sessionPath(projectRoot, id);
  if (!existsSync(path)) return false;
  unlinkSync(path);
  return true;
}

export function addTurn(session: Session, turn: AgentTurn): void {
  session.turns.push(turn);
}

export function getLatestSession(projectRoot: string): Session | null {
  const sessions = listSessions(projectRoot);
  if (sessions.length === 0) return null;
  return loadSession(projectRoot, sessions[0].id);
}
