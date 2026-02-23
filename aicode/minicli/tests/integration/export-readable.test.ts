import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";
import { MessageRecord, SessionRecord } from "../../src/repository";

function createInMemorySessionRepository(initialTitles: string[] = []): {
  sessions: SessionRecord[];
  listSessions: () => SessionRecord[];
  createSession: (title: string) => SessionRecord;
} {
  const sessions: SessionRecord[] = initialTitles.map((title, index) => ({
    id: index + 1,
    title,
    createdAt: "2026-02-23 00:00:00",
    updatedAt: "2026-02-23 00:00:00"
  }));
  return {
    sessions,
    listSessions: () => sessions.map((x) => ({ ...x })),
    createSession: (title: string) => {
      const created: SessionRecord = {
        id: sessions.length + 1,
        title,
        createdAt: "2026-02-23 00:00:00",
        updatedAt: "2026-02-23 00:00:00"
      };
      sessions.push(created);
      return created;
    }
  };
}

function createInMemoryMessageRepository(
  sessions: { sessions: SessionRecord[] }
): {
  createMessage: (input: {
    sessionId: number;
    role: "system" | "user" | "assistant" | "tool";
    content: string;
  }) => MessageRecord;
  listMessagesBySession: (sessionId: number) => MessageRecord[];
} {
  const messages: MessageRecord[] = [];
  return {
    createMessage: (input) => {
      const created: MessageRecord = {
        id: messages.length + 1,
        sessionId: input.sessionId,
        role: input.role,
        content: input.content,
        createdAt: `2026-02-23 00:00:0${messages.length}`
      };
      messages.push(created);
      const session = sessions.sessions.find((item) => item.id === input.sessionId);
      if (session) {
        session.updatedAt = created.createdAt;
      }
      return created;
    },
    listMessagesBySession: (sessionId: number) =>
      messages
        .filter((item) => item.sessionId === sessionId)
        .map((item) => ({ ...item }))
  };
}

test("/export writes readable markdown file", async () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-export-int-"));
  const prevCwd = process.cwd();
  const writes: string[] = [];
  const errors: string[] = [];
  try {
    process.chdir(tmpRoot);
    const sessions = createInMemorySessionRepository();
    const messages = createInMemoryMessageRepository(sessions);
    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH,
      {
        sessionRepository: {
          createSession: sessions.createSession,
          listSessions: sessions.listSessions
        },
        messageRepository: {
          createMessage: messages.createMessage,
          listMessagesBySession: messages.listMessagesBySession
        }
      }
    );

    await session.onLine("/new sprint-export");
    await session.onLine("demo content");
    await session.onLine("/export --format md --out exports/chat");

    assert.equal(errors.join(""), "");
    const outputPath = path.join(tmpRoot, "exports", "chat.md");
    assert.equal(fs.existsSync(outputPath), true);
    const content = fs.readFileSync(outputPath, "utf8");
    assert.match(content, /MiniCLI Session Export/);
    assert.match(content, /sessionTitle: sprint-export/);
    assert.match(content, /demo content/);
    assert.match(content, /mock\(mock-mini\): demo content/);
    assert.match(writes.join(""), /\[export\] wrote 2 message\(s\) to/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});
