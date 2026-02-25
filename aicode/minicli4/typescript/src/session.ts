import fs from "node:fs";
import path from "node:path";

export type MessageRole = "system" | "user" | "assistant" | "tool";

export interface SessionMessage {
  role: MessageRole;
  content: string;
  created_at: string;
  tool_call_id?: string;
  name?: string;
}

export interface SessionRecord {
  session_id: string;
  title: string;
  messages: SessionMessage[];
  updated_at: string;
}

function sessionsDir(stateDir: string): string {
  return path.join(stateDir, "sessions");
}

function currentPath(stateDir: string): string {
  return path.join(stateDir, "current.txt");
}

function sessionPath(stateDir: string, id: string): string {
  return path.join(sessionsDir(stateDir), `${id}.json`);
}

function nowIso(): string {
  return new Date().toISOString();
}

export class SessionStore {
  constructor(private readonly stateDir: string) {}

  private ensure(): void {
    fs.mkdirSync(sessionsDir(this.stateDir), { recursive: true });
  }

  list(): SessionRecord[] {
    this.ensure();
    const files = fs.readdirSync(sessionsDir(this.stateDir)).filter((x) => x.endsWith(".json"));
    return files.map((name) => {
      const full = path.join(sessionsDir(this.stateDir), name);
      const rec = JSON.parse(fs.readFileSync(full, "utf8")) as SessionRecord;
      return rec;
    }).sort((a, b) => b.updated_at.localeCompare(a.updated_at));
  }

  create(id?: string): SessionRecord {
    this.ensure();
    const sessionId = id && id.trim().length > 0 ? id.trim() : `session-${Date.now()}`;
    const rec: SessionRecord = {
      session_id: sessionId,
      title: sessionId,
      messages: [],
      updated_at: nowIso()
    };
    this.save(rec);
    this.setCurrent(sessionId);
    return rec;
  }

  load(id: string): SessionRecord {
    const full = sessionPath(this.stateDir, id);
    if (!fs.existsSync(full)) {
      throw new Error(`session not found: ${id}`);
    }
    return JSON.parse(fs.readFileSync(full, "utf8")) as SessionRecord;
  }

  save(rec: SessionRecord): void {
    this.ensure();
    rec.updated_at = nowIso();
    fs.writeFileSync(sessionPath(this.stateDir, rec.session_id), JSON.stringify(rec, null, 2), "utf8");
  }

  setCurrent(id: string): void {
    this.ensure();
    fs.writeFileSync(currentPath(this.stateDir), id, "utf8");
  }

  current(): SessionRecord {
    this.ensure();
    const marker = currentPath(this.stateDir);
    if (!fs.existsSync(marker)) {
      return this.create("default");
    }
    const id = fs.readFileSync(marker, "utf8").trim();
    try {
      return this.load(id);
    } catch {
      return this.create("default");
    }
  }

  delete(id: string): boolean {
    const full = sessionPath(this.stateDir, id);
    if (!fs.existsSync(full)) {
      return false;
    }
    fs.unlinkSync(full);
    return true;
  }

  rename(oldId: string, newId: string): boolean {
    const oldPath = sessionPath(this.stateDir, oldId);
    const newPath = sessionPath(this.stateDir, newId);
    if (!fs.existsSync(oldPath) || fs.existsSync(newPath)) {
      return false;
    }
    const rec = this.load(oldId);
    rec.session_id = newId;
    rec.title = newId;
    fs.unlinkSync(oldPath);
    this.save(rec);
    if (this.current().session_id === oldId) {
      this.setCurrent(newId);
    }
    return true;
  }
}
