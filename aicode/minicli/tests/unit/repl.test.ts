import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import {
  acceptReplTabCompletion,
  addContextFile,
  buildContextSystemMessage,
  buildDefaultSessionTitle,
  classifyReplInput,
  completeReplCommandPrefix,
  createReplReadlineCompleter,
  createRuntimeProvider,
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  DEFAULT_MAX_HISTORY_PREVIEW_LENGTH,
  dropContextFiles,
  EMPTY_REPLY_PLACEHOLDER,
  formatCompletionCandidates,
  formatContextFileList,
  formatHistoryList,
  formatSessionList,
  formatSessionTimestamp,
  HELP_TEXT,
  incrementCompletionUsageFrequency,
  isBinaryContent,
  MAX_COMPLETION_USAGE_FREQUENCY,
  matchReplCommand,
  normalizeContextFilePath,
  OutputBuffer,
  parseHistoryCommandArgs,
  parseNewSessionTitle,
  parseSwitchCommandArgs,
  parseSessionsCommandArgs,
  parseReplLine,
  resolveInlineTabCompletions,
  renderAssistantReply,
  resolveUniqueSessionTitle,
  shouldUseTerminalMode,
  sortCompletionCandidatesByUsageFrequency,
  sortSessionsByRecent,
  validateAddContextFile
} from "../../src/repl";
import { LLMProvider } from "../../src/provider";
import { MessageRecord, SessionRecord } from "../../src/repository";

function createInMemorySessionRepository(initialTitles: string[] = []): {
  sessions: SessionRecord[];
  listSessions: () => SessionRecord[];
  createSession: (title: string) => SessionRecord;
} {
  const sessions: SessionRecord[] = initialTitles.map((title, index) => ({
    id: index + 1,
    title,
    createdAt: "2026-02-13 00:00:00",
    updatedAt: "2026-02-13 00:00:00"
  }));
  return {
    sessions,
    listSessions: () => sessions.map((x) => ({ ...x })),
    createSession: (title: string) => {
      const created: SessionRecord = {
        id: sessions.length + 1,
        title,
        createdAt: "2026-02-13 00:00:00",
        updatedAt: "2026-02-13 00:00:00"
      };
      sessions.push(created);
      return created;
    }
  };
}

function createInMemoryMessageRepository(
  sessions: { sessions: SessionRecord[] }
): {
  messages: MessageRecord[];
  createMessage: (input: {
    sessionId: number;
    role: "system" | "user" | "assistant" | "tool";
    content: string;
  }) => MessageRecord;
  listMessagesBySession: (sessionId: number) => MessageRecord[];
} {
  const messages: MessageRecord[] = [];
  return {
    messages,
    createMessage: (input) => {
      const created: MessageRecord = {
        id: messages.length + 1,
        sessionId: input.sessionId,
        role: input.role,
        content: input.content,
        createdAt: `2026-02-13 00:00:0${messages.length}`
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

test("parseReplLine parses regular input", () => {
  const parsed = parseReplLine("hello world");
  assert.deepEqual(parsed, {
    kind: "message",
    text: "hello world",
    truncated: false
  });
});

test("parseReplLine ignores empty and whitespace-only input", () => {
  assert.deepEqual(parseReplLine(""), {
    kind: "empty",
    text: "",
    truncated: false
  });
  assert.deepEqual(parseReplLine("    "), {
    kind: "empty",
    text: "",
    truncated: false
  });
});

test("parseReplLine truncates overlong input", () => {
  const input = "x".repeat(DEFAULT_MAX_INPUT_LENGTH + 20);
  const parsed = parseReplLine(input);
  assert.equal(parsed.kind, "message");
  assert.equal(parsed.text.length, DEFAULT_MAX_INPUT_LENGTH);
  assert.equal(parsed.truncated, true);
});

test("classifyReplInput classifies empty, command, and message", () => {
  assert.deepEqual(classifyReplInput("   "), { kind: "empty" });
  assert.deepEqual(classifyReplInput("/help"), {
    kind: "command",
    command: { kind: "help" }
  });
  assert.deepEqual(classifyReplInput("hello"), {
    kind: "message",
    text: "hello",
    truncated: false
  });
});

test("renderAssistantReply uses fallback for empty content", () => {
  const rendered = renderAssistantReply("   ");
  assert.equal(rendered.text, EMPTY_REPLY_PLACEHOLDER);
  assert.equal(rendered.usedFallback, true);
  assert.equal(rendered.truncated, false);
});

test("renderAssistantReply truncates overlong content", () => {
  const rendered = renderAssistantReply("abcdefghij", 5);
  assert.equal(rendered.text, "abcde...[truncated]");
  assert.equal(rendered.truncated, true);
  assert.equal(rendered.usedFallback, false);
});

test("createRuntimeProvider selects mock when apiKey is absent", () => {
  const provider = createRuntimeProvider({
    model: "glm-4",
    timeoutMs: 30000
  });
  assert.equal(provider.id, "mock");
});

test("createRuntimeProvider selects GLM provider when apiKey exists", () => {
  const provider = createRuntimeProvider({
    model: "glm-4",
    timeoutMs: 30000,
    apiKey: "sk-test"
  });
  assert.equal(provider.id, "glm-openai-compatible");
});

test("repl session ignores empty line and sends non-empty line to mock provider", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: (message) => errors.push(message)
  });

  const shouldExitFromEmpty = await session.onLine("   ");
  const shouldExitFromEcho = await session.onLine("ping");

  assert.deepEqual(errors, []);
  assert.equal(shouldExitFromEmpty, false);
  assert.equal(shouldExitFromEcho, false);
  assert.equal(writes.join(""), "mock(mock-mini): ping\n");
});

test("repl session builds request using configured model and user input", async () => {
  const writes: string[] = [];
  let capturedModel = "";
  let capturedMessages: Array<{ role: string; content: string }> = [];
  const provider: LLMProvider = {
    id: "capture",
    complete: async (request) => {
      capturedModel = request.model;
      capturedMessages = request.messages;
      return {
        message: {
          role: "assistant",
          content: "ok"
        }
      };
    }
  };

  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      provider,
      config: {
        model: "glm-4-air",
        timeoutMs: 30000
      }
    }
  );

  await session.onLine("hello request");

  assert.equal(capturedModel, "glm-4-air");
  assert.deepEqual(capturedMessages, [{ role: "user", content: "hello request" }]);
  assert.match(writes.join(""), /ok/);
});

test("matchReplCommand matches help and exit commands", () => {
  assert.deepEqual(matchReplCommand("/help"), { kind: "help" });
  assert.deepEqual(matchReplCommand("/exit"), { kind: "exit" });
  assert.deepEqual(matchReplCommand("/quit"), { kind: "exit" });
  assert.deepEqual(matchReplCommand("/login token"), {
    kind: "login",
    args: ["token"]
  });
  assert.deepEqual(matchReplCommand("/model"), {
    kind: "model",
    args: []
  });
  assert.deepEqual(matchReplCommand("/new"), {
    kind: "new",
    args: []
  });
  assert.deepEqual(matchReplCommand("/new sprint planning"), {
    kind: "new",
    args: ["sprint", "planning"]
  });
  assert.deepEqual(matchReplCommand("/sessions --limit 2"), {
    kind: "sessions",
    args: ["--limit", "2"]
  });
  assert.deepEqual(matchReplCommand("/switch #2"), {
    kind: "switch",
    args: ["#2"]
  });
  assert.deepEqual(matchReplCommand("/history --limit 3"), {
    kind: "history",
    args: ["--limit", "3"]
  });
  assert.deepEqual(matchReplCommand("/run pwd"), {
    kind: "run",
    args: ["pwd"]
  });
  assert.deepEqual(matchReplCommand("/add src/repl.ts"), {
    kind: "add",
    args: ["src/repl.ts"]
  });
  assert.deepEqual(matchReplCommand("/drop 1"), {
    kind: "drop",
    args: ["1"]
  });
});

test("completeReplCommandPrefix matches slash prefixes", () => {
  assert.deepEqual(completeReplCommandPrefix("/mo"), ["/model"]);
});

test("completeReplCommandPrefix returns all commands for empty slash prefix", () => {
  assert.deepEqual(completeReplCommandPrefix("/"), [
    "/help",
    "/exit",
    "/login",
    "/model",
    "/new",
    "/sessions",
    "/switch",
    "/history",
    "/run",
    "/add",
    "/drop"
  ]);
});

test("completeReplCommandPrefix returns empty list for no matches", () => {
  assert.deepEqual(completeReplCommandPrefix("/zzz"), []);
});

test("completeReplCommandPrefix supports alias lookup and returns canonical name", () => {
  assert.deepEqual(completeReplCommandPrefix("/qu"), ["/exit"]);
});

test("completeReplCommandPrefix keeps registry order stable", () => {
  assert.deepEqual(completeReplCommandPrefix("/"), [
    "/help",
    "/exit",
    "/login",
    "/model",
    "/new",
    "/sessions",
    "/switch",
    "/history",
    "/run",
    "/add",
    "/drop"
  ]);
});

test("completeReplCommandPrefix prioritizes higher frequency commands", () => {
  assert.deepEqual(
    completeReplCommandPrefix("/s", undefined, {
      "/switch": 3,
      "/sessions": 1
    }),
    ["/switch", "/sessions"]
  );
});

test("completeReplCommandPrefix is case-sensitive", () => {
  assert.deepEqual(completeReplCommandPrefix("/MO"), []);
});

test("sortCompletionCandidatesByUsageFrequency keeps stable order when same frequency", () => {
  assert.deepEqual(
    sortCompletionCandidatesByUsageFrequency(
      ["/sessions", "/switch", "/search"],
      {
        "/sessions": 2,
        "/switch": 2,
        "/search": 2
      }
    ),
    ["/sessions", "/switch", "/search"]
  );
});

test("incrementCompletionUsageFrequency initializes and accumulates", () => {
  const usage: Record<string, number> = {};
  assert.equal(incrementCompletionUsageFrequency(usage, "/switch"), 1);
  assert.equal(incrementCompletionUsageFrequency(usage, "/switch"), 2);
  assert.equal(usage["/switch"], 2);
});

test("incrementCompletionUsageFrequency caps overflow at configured maximum", () => {
  const usage: Record<string, number> = {
    "/switch": MAX_COMPLETION_USAGE_FREQUENCY
  };
  assert.equal(
    incrementCompletionUsageFrequency(usage, "/switch"),
    MAX_COMPLETION_USAGE_FREQUENCY
  );
  assert.equal(usage["/switch"], MAX_COMPLETION_USAGE_FREQUENCY);
});

test("formatCompletionCandidates renders candidate list", () => {
  assert.equal(
    formatCompletionCandidates(["/model", "/mock"]),
    "Completions:\n/model\n/mock\n"
  );
});

test("acceptReplTabCompletion accepts single candidate and appends trailing space", () => {
  const result = acceptReplTabCompletion("/mo", 3);
  assert.equal(result.accepted, true);
  assert.deepEqual(result.candidates, ["/model"]);
  assert.equal(result.line, "/model ");
  assert.equal(result.cursor, 7);
});

test("acceptReplTabCompletion leaves line unchanged when multiple candidates exist", () => {
  const result = acceptReplTabCompletion("/s", 2);
  assert.equal(result.accepted, false);
  assert.deepEqual(result.candidates, ["/sessions", "/switch"]);
  assert.equal(result.line, "/s");
  assert.equal(result.cursor, 2);
});

test("acceptReplTabCompletion leaves line unchanged when no candidate exists", () => {
  const result = acceptReplTabCompletion("/zz", 3);
  assert.equal(result.accepted, false);
  assert.deepEqual(result.candidates, []);
  assert.equal(result.line, "/zz");
  assert.equal(result.cursor, 3);
});

test("acceptReplTabCompletion replaces token using cursor position inside first token", () => {
  const result = acceptReplTabCompletion("/mo target", 3);
  assert.equal(result.accepted, true);
  assert.equal(result.line, "/model target");
  assert.equal(result.cursor, 6);
});

test("acceptReplTabCompletion is no-op after completed token and trailing space", () => {
  const result = acceptReplTabCompletion("/model ", 7);
  assert.equal(result.accepted, false);
  assert.equal(result.line, "/model ");
  assert.equal(result.cursor, 7);
});

test("resolveInlineTabCompletions handles consecutive tabs", () => {
  assert.equal(resolveInlineTabCompletions("/mo\t\t"), "/model ");
});

test("resolveInlineTabCompletions cycles down through candidates", () => {
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[B\t"), "/switch ");
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[B\u001b[B\t"), "/sessions ");
});

test("resolveInlineTabCompletions cycles up through candidates", () => {
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[A\t"), "/switch ");
});

test("resolveInlineTabCompletions clears candidate navigation on Esc", () => {
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[B\u001b\t"), "/s");
});

test("resolveInlineTabCompletions ignores arrow navigation when no candidates exist", () => {
  assert.equal(resolveInlineTabCompletions("/zz\u001b[B\t"), "/zz");
});

test("resolveInlineTabCompletions resets completion focus after regular typing", () => {
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[Bx\t"), "/sx");
});

test("resolveInlineTabCompletions resets state after accepting a candidate", () => {
  assert.equal(resolveInlineTabCompletions("/s\t\u001b[B\t\u001b[A\t"), "/switch ");
});

test("createReplReadlineCompleter returns single hit for /mo", () => {
  const completer = createReplReadlineCompleter();
  const [hits, token] = completer("/mo");
  assert.deepEqual(hits, ["/model"]);
  assert.equal(token, "/mo");
});

test("createReplReadlineCompleter returns multiple hits for /s", () => {
  const completer = createReplReadlineCompleter();
  const [hits, token] = completer("/s");
  assert.deepEqual(hits, ["/sessions", "/switch"]);
  assert.equal(token, "/s");
});

test("shouldUseTerminalMode returns true when either input or output is TTY", () => {
  assert.equal(
    shouldUseTerminalMode({
      input: { isTTY: true } as unknown as NodeJS.ReadableStream,
      output: { isTTY: false } as unknown as NodeJS.WritableStream,
      stdout: () => {},
      stderr: () => {}
    }),
    true
  );
  assert.equal(
    shouldUseTerminalMode({
      input: { isTTY: false } as unknown as NodeJS.ReadableStream,
      output: { isTTY: true } as unknown as NodeJS.WritableStream,
      stdout: () => {},
      stderr: () => {}
    }),
    true
  );
  assert.equal(
    shouldUseTerminalMode({
      input: { isTTY: false } as unknown as NodeJS.ReadableStream,
      output: { isTTY: false } as unknown as NodeJS.WritableStream,
      stdout: () => {},
      stderr: () => {}
    }),
    false
  );
});

test("parseNewSessionTitle joins args and trims spaces", () => {
  assert.equal(parseNewSessionTitle(["my", "chat"]), "my chat");
  assert.equal(parseNewSessionTitle([]), undefined);
  assert.equal(parseNewSessionTitle(["   "]), undefined);
});

test("parseSessionsCommandArgs parses pagination and query", () => {
  assert.deepEqual(
    parseSessionsCommandArgs(["--limit", "2", "--offset", "1", "--q", "plan"]),
    {
      options: {
        limit: 2,
        offset: 1,
        query: "plan"
      }
    }
  );
  assert.deepEqual(parseSessionsCommandArgs([]), {
    options: {
      offset: 0
    }
  });
});

test("parseSessionsCommandArgs rejects invalid values", () => {
  assert.match(
    parseSessionsCommandArgs(["--limit", "0"]).error ?? "",
    /positive integer/
  );
  assert.match(
    parseSessionsCommandArgs(["--offset", "-1"]).error ?? "",
    /non-negative integer/
  );
  assert.match(parseSessionsCommandArgs(["--q"]).error ?? "", /Missing value/);
});

test("parseSwitchCommandArgs supports id and index targets", () => {
  assert.deepEqual(parseSwitchCommandArgs(["#4"]), {
    target: {
      mode: "id",
      value: 4
    }
  });
  assert.deepEqual(parseSwitchCommandArgs(["2"]), {
    target: {
      mode: "index",
      value: 2
    }
  });
});

test("parseSwitchCommandArgs validates malformed targets", () => {
  assert.match(parseSwitchCommandArgs([]).error ?? "", /Missing switch target/);
  assert.match(
    parseSwitchCommandArgs(["#0"]).error ?? "",
    /Session id must be a positive integer/
  );
  assert.match(
    parseSwitchCommandArgs(["0"]).error ?? "",
    /Session index must be a positive integer/
  );
});

test("parseHistoryCommandArgs parses optional limit", () => {
  assert.deepEqual(parseHistoryCommandArgs([]), {
    options: {
      offset: 0,
      audit: false
    }
  });
  assert.deepEqual(parseHistoryCommandArgs(["--limit", "2"]), {
    options: {
      limit: 2,
      offset: 0,
      audit: false
    }
  });
  assert.deepEqual(
    parseHistoryCommandArgs(["--audit", "--status", "rejected", "--offset", "1"]),
    {
      options: {
        offset: 1,
        audit: true,
        approvalStatus: "rejected"
      }
    }
  );
});

test("parseHistoryCommandArgs validates invalid limit", () => {
  assert.match(parseHistoryCommandArgs(["--limit"]).error ?? "", /Missing value/);
  assert.match(
    parseHistoryCommandArgs(["--limit", "0"]).error ?? "",
    /positive integer/
  );
  assert.match(
    parseHistoryCommandArgs(["--offset", "-1"]).error ?? "",
    /non-negative integer/
  );
  assert.match(
    parseHistoryCommandArgs(["--status", "oops"]).error ?? "",
    /must be one of/
  );
  assert.match(
    parseHistoryCommandArgs(["oops"]).error ?? "",
    /Unknown argument/
  );
});

test("formatHistoryList truncates overlong content and preserves role labels", () => {
  const output = formatHistoryList(
    [
      {
        role: "user",
        content: "hello"
      },
      {
        role: "assistant",
        content: "x".repeat(DEFAULT_MAX_HISTORY_PREVIEW_LENGTH + 5)
      }
    ],
    DEFAULT_MAX_HISTORY_PREVIEW_LENGTH
  );

  assert.match(output, /\[1\] user: hello/);
  assert.match(output, /\[2\] assistant: x+\.\.\.\[truncated\]/);
});

test("formatSessionTimestamp renders ISO-like second precision", () => {
  const date = new Date("2026-02-13T00:12:34.999Z");
  assert.equal(formatSessionTimestamp(date), "2026-02-13 00:12:34");
  assert.equal(
    buildDefaultSessionTitle(date),
    "New Session 2026-02-13 00:12:34"
  );
});

test("resolveUniqueSessionTitle appends numeric suffix for duplicates", () => {
  assert.equal(resolveUniqueSessionTitle("Work", []), "Work");
  assert.equal(resolveUniqueSessionTitle("Work", ["Work"]), "Work (2)");
  assert.equal(
    resolveUniqueSessionTitle("Work", ["Work", "Work (2)"]),
    "Work (3)"
  );
});

test("sortSessionsByRecent sorts by updatedAt desc then id desc", () => {
  const sorted = sortSessionsByRecent([
    {
      id: 1,
      title: "alpha",
      createdAt: "2026-02-13 00:00:00",
      updatedAt: "2026-02-13 00:00:00"
    },
    {
      id: 3,
      title: "charlie",
      createdAt: "2026-02-13 00:00:00",
      updatedAt: "2026-02-14 00:00:00"
    },
    {
      id: 2,
      title: "bravo",
      createdAt: "2026-02-13 00:00:00",
      updatedAt: "2026-02-14 00:00:00"
    }
  ]);
  assert.deepEqual(
    sorted.map((item) => item.id),
    [3, 2, 1]
  );
});

test("formatSessionList renders marker and numbered rows", () => {
  const output = formatSessionList(
    [
      {
        id: 2,
        title: "second",
        createdAt: "2026-02-13 00:00:00",
        updatedAt: "2026-02-14 00:00:00"
      },
      {
        id: 1,
        title: "first",
        createdAt: "2026-02-13 00:00:00",
        updatedAt: "2026-02-13 00:00:00"
      }
    ],
    1
  );
  assert.match(output, /  \[1\] #2 second/);
  assert.match(output, /\* \[2\] #1 first/);
});

test("matchReplCommand handles unknown command and non-command input", () => {
  assert.deepEqual(matchReplCommand("/unknown"), {
    kind: "unknown",
    token: "/unknown"
  });
  assert.deepEqual(matchReplCommand("hello"), { kind: "none" });
});

test("command matching is case-sensitive", () => {
  assert.deepEqual(matchReplCommand("/Help"), {
    kind: "unknown",
    token: "/Help"
  });
  assert.deepEqual(matchReplCommand("/EXIT"), {
    kind: "unknown",
    token: "/EXIT"
  });
});

test("repl session prints help content", async () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = await session.onLine("/help");

  assert.equal(shouldExit, false);
  assert.equal(writes.join(""), HELP_TEXT);
});

test("repl /new uses default timestamp title for empty input", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      now: () => new Date("2026-02-13T00:00:00.000Z")
    }
  );

  await session.onLine("/new");

  assert.match(writes.join(""), /Switched to session #1: New Session 2026-02-13 00:00:00/);
});

test("repl /new applies duplicate title strategy", async () => {
  const sessionRepo = createInMemorySessionRepository(["Plan", "Plan (2)"]);
  const writes: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/new Plan");

  assert.match(writes.join(""), /Switched to session #3: Plan \(3\)/);
});

test("repl /new updates current session pointer for subsequent writes", async () => {
  const writes: string[] = [];
  const persistedMessages: MessageRecord[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const provider: LLMProvider = {
    id: "ok",
    complete: async () => ({
      message: {
        role: "assistant",
        content: "done"
      }
    })
  };
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      provider,
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: (input: {
          sessionId: number;
          role: "system" | "user" | "assistant" | "tool";
          content: string;
        }) => {
          const created: MessageRecord = {
            id: persistedMessages.length,
            sessionId: input.sessionId,
            role: input.role,
            content: input.content,
            createdAt: "2026-02-13 00:00:00"
          };
          persistedMessages.push(created);
          return created;
        },
        listMessagesBySession: (sessionId: number) =>
          persistedMessages
            .filter((item) => item.sessionId === sessionId)
            .map((item) => ({ ...item }))
      }
    }
  );

  await session.onLine("/new first");
  await session.onLine("/new second");
  await session.onLine("hello");

  assert.match(writes.join(""), /Switched to session #2: second/);
  assert.deepEqual(
    persistedMessages.map((item) => item.sessionId),
    [2, 2]
  );
});

test("repl /sessions prints empty state when no sessions exist", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/sessions");

  assert.equal(writes.join(""), "No sessions.\n");
});

test("repl /sessions marks current session and keeps recent-first order", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/new alpha");
  await session.onLine("/new beta");
  await session.onLine("/sessions");

  const all = writes.join("");
  assert.match(all, /\* \[1\] #2 beta/);
  assert.match(all, /  \[2\] #1 alpha/);
});

test("repl /sessions supports pagination and filtering", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/new plan-alpha");
  await session.onLine("/new chat");
  await session.onLine("/new plan-beta");
  writes.length = 0;

  await session.onLine("/sessions --q plan --limit 1 --offset 1");

  const output = writes.join("");
  assert.match(output, /#1 plan-alpha/);
  assert.doesNotMatch(output, /plan-beta/);
});

test("repl /sessions reports bad arguments", async () => {
  const errors: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/sessions --limit 0");

  assert.match(errors.join(""), /Usage: \/sessions \[--limit N\] \[--offset N\] \[--q keyword\]/);
});

test("repl /switch switches by id target", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  sessionRepo.createSession("alpha");
  sessionRepo.createSession("beta");
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch #1");

  assert.match(writes.join(""), /Switched to session #1: alpha/);
});

test("repl /switch switches by index target", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  sessionRepo.createSession("alpha");
  sessionRepo.createSession("beta");
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch 1");

  assert.match(writes.join(""), /Switched to session #2: beta/);
});

test("repl /switch reports missing target argument", async () => {
  const errors: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch");

  assert.match(errors.join(""), /Usage: \/switch <#id\|index>/);
});

test("repl /switch reports out-of-range index boundary", async () => {
  const errors: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  sessionRepo.createSession("solo");
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch 2");

  assert.match(errors.join(""), /target session does not exist/);
});

test("repl /switch reports non-existent id target", async () => {
  const errors: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  sessionRepo.createSession("solo");
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch #8");

  assert.match(errors.join(""), /target session does not exist/);
});

test("repl /switch reports repeated switch without state changes", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  sessionRepo.createSession("alpha");
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      }
    }
  );

  await session.onLine("/switch #1");
  await session.onLine("/switch #1");

  assert.match(writes.join(""), /Already in session #1: alpha/);
});

test("repl /history prints empty state for current session with no messages", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const messageRepo = createInMemoryMessageRepository(sessionRepo);
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: messageRepo.createMessage,
        listMessagesBySession: messageRepo.listMessagesBySession
      }
    }
  );

  await session.onLine("/new alpha");
  writes.length = 0;
  await session.onLine("/history");

  assert.equal(writes.join(""), "No history.\n");
});

test("repl /history preserves message order and role labels", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const messageRepo = createInMemoryMessageRepository(sessionRepo);
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: messageRepo.createMessage,
        listMessagesBySession: messageRepo.listMessagesBySession
      }
    }
  );

  await session.onLine("/new alpha");
  await session.onLine("first");
  await session.onLine("second");
  writes.length = 0;
  await session.onLine("/history");

  const output = writes.join("");
  assert.match(output, /\[1\] user: first/);
  assert.match(output, /\[2\] assistant: mock\(mock-mini\): first/);
  assert.match(output, /\[3\] user: second/);
  assert.match(output, /\[4\] assistant: mock\(mock-mini\): second/);
});

test("repl /history applies limit to tail messages", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const messageRepo = createInMemoryMessageRepository(sessionRepo);
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: messageRepo.createMessage,
        listMessagesBySession: messageRepo.listMessagesBySession
      }
    }
  );

  await session.onLine("/new alpha");
  await session.onLine("first");
  await session.onLine("second");
  writes.length = 0;
  await session.onLine("/history --limit 2");

  const output = writes.join("");
  assert.doesNotMatch(output, /\[1\] user: first/);
  assert.match(output, /\[1\] user: second/);
  assert.match(output, /\[2\] assistant: mock\(mock-mini\): second/);
});

test("repl /history truncates overlong message content", async () => {
  const writes: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const messageRepo = createInMemoryMessageRepository(sessionRepo);
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: messageRepo.createMessage,
        listMessagesBySession: messageRepo.listMessagesBySession
      }
    }
  );

  await session.onLine("/new alpha");
  await session.onLine("x".repeat(DEFAULT_MAX_HISTORY_PREVIEW_LENGTH + 20));
  writes.length = 0;
  await session.onLine("/history");

  assert.match(writes.join(""), /\.\.\.\[truncated\]/);
});

test("repl /history reports invalid limit value", async () => {
  const errors: string[] = [];
  const sessionRepo = createInMemorySessionRepository();
  const messageRepo = createInMemoryMessageRepository(sessionRepo);
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      sessionRepository: {
        listSessions: sessionRepo.listSessions,
        createSession: sessionRepo.createSession
      },
      messageRepository: {
        createMessage: messageRepo.createMessage,
        listMessagesBySession: messageRepo.listMessagesBySession
      }
    }
  );

  await session.onLine("/new alpha");
  await session.onLine("/history --limit 0");

  assert.match(
    errors.join(""),
    /Usage: \/history \[--limit N\] \[--offset N\] \[--audit\] \[--status <not_required\|approved\|rejected\|timeout>\]/
  );
});

test("repl /history --audit shows run audit records", async () => {
  const writes: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      runAuditRepository: {
        recordAudit: () => ({
          id: 1,
          command: "pwd",
          riskLevel: "low",
          approvalStatus: "not_required",
          executed: true,
          exitCode: 0,
          stdout: "C:/workspace",
          stderr: "",
          createdAt: "2026-02-13 00:00:00"
        }),
        listAudits: () => [
          {
            id: 1,
            command: "pwd",
            riskLevel: "low",
            approvalStatus: "not_required",
            executed: true,
            exitCode: 0,
            stdout: "C:/workspace",
            stderr: "",
            createdAt: "2026-02-13 00:00:00"
          }
        ]
      },
      executeRunCommand: () => ({
        ok: true,
        stdout: "C:/workspace",
        stderr: "",
        exitCode: 0,
        stdoutTruncated: false,
        stderrTruncated: false
      })
    }
  );

  await session.onLine("/run pwd");
  writes.length = 0;
  await session.onLine("/history --audit");

  const output = writes.join("");
  assert.match(output, /status=not_required/);
  assert.match(output, /risk=low/);
  assert.match(output, /cmd=pwd/);
});

test("repl /run executes medium-risk command after confirmation", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const executedCommands: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "ok",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  await session.onLine("yes");

  assert.equal(executedCommands.length, 1);
  assert.equal(executedCommands[0], "pwd | wc -c");
  assert.equal(writes.join(""), "ok\n");
  assert.match(errors.join(""), /\[run:confirm\] medium-risk command requires confirmation\./);
});

test("repl /run rejects medium-risk command when denied", async () => {
  const errors: string[] = [];
  const executedCommands: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  await session.onLine("no");

  assert.equal(executedCommands.length, 0);
  assert.match(errors.join(""), /\[run:confirm\] command cancelled\./);
});

test("repl /run confirmation times out and rejects late approval", async () => {
  const errors: string[] = [];
  const executedCommands: string[] = [];
  let nowMs = 0;
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      now: () => new Date(nowMs),
      runConfirmationTimeoutMs: 1000,
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  nowMs = 2001;
  await session.onLine("yes");

  assert.equal(executedCommands.length, 0);
  assert.match(errors.join(""), /\[run:confirm\] confirmation timed out; command cancelled\./);
});

test("repl /run confirmation retries on invalid input", async () => {
  const errors: string[] = [];
  const executedCommands: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  await session.onLine("maybe");
  await session.onLine("yes");

  assert.equal(executedCommands.length, 1);
  assert.match(errors.join(""), /\[run:confirm\] invalid input\. Reply yes or no\./);
});

test("repl /run high-risk confirmation includes enhanced danger prompt", async () => {
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH
  );

  await session.onLine("/run rm -rf .");

  assert.match(errors.join(""), /DANGER: high-risk command requires explicit approval\./);
});

test("repl /run confirmation state is recycled after cancellation", async () => {
  const errors: string[] = [];
  const executedCommands: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  await session.onLine("no");
  await session.onLine("/run pwd");

  assert.equal(executedCommands.length, 1);
  assert.equal(executedCommands[0], "pwd");
});

test("repl /run validates missing command argument", async () => {
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH
  );

  await session.onLine("/run");

  assert.match(errors.join(""), /\[run:error\] Usage: \/run <command>/);
});

test("normalizeContextFilePath resolves and normalizes input path", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const normalized = normalizeContextFilePath(".\\src\\..\\src\\repl.ts", cwd);
  assert.equal(
    normalized,
    path.normalize(path.join("C:", "workspace", "minicli", "src", "repl.ts"))
  );
});

test("addContextFile deduplicates repeated file additions", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-add-dedupe-"));
  try {
    const target = path.join(tmpRoot, "note.txt");
    fs.writeFileSync(target, "hello\n", "utf8");

    const files: string[] = [];
    const first = addContextFile(files, target, tmpRoot);
    const second = addContextFile(files, target, tmpRoot);

    assert.equal(first.ok, true);
    assert.equal(first.added, true);
    assert.equal(second.ok, true);
    assert.equal(second.added, false);
    assert.equal(files.length, 1);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("validateAddContextFile reports missing path", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-add-missing-"));
  try {
    const missing = path.join(tmpRoot, "missing.txt");
    const result = validateAddContextFile(missing);
    assert.equal(result.ok, false);
    assert.match(result.error, /File does not exist/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("validateAddContextFile rejects directory path input", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-add-dir-"));
  try {
    const result = validateAddContextFile(tmpRoot);
    assert.equal(result.ok, false);
    assert.match(result.error, /Path is a directory/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("validateAddContextFile rejects binary content", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-add-binary-"));
  try {
    const target = path.join(tmpRoot, "binary.bin");
    fs.writeFileSync(target, Buffer.from([0, 159, 146, 150]));
    assert.equal(isBinaryContent(fs.readFileSync(target)), true);
    const result = validateAddContextFile(target);
    assert.equal(result.ok, false);
    assert.match(result.error, /Binary file is not supported/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("validateAddContextFile rejects invalid utf-8 text file", () => {
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-add-encoding-"));
  try {
    const target = path.join(tmpRoot, "broken.txt");
    fs.writeFileSync(target, Buffer.from([0xc3, 0x28]));
    const result = validateAddContextFile(target);
    assert.equal(result.ok, false);
    assert.match(result.error, /not valid UTF-8/);
  } finally {
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("formatContextFileList renders indexed context entries", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const filePath = path.join(cwd, "src", "repl.ts");
  const output = formatContextFileList([filePath], cwd);
  assert.equal(output, `Context files:\n[1] ${path.join("src", "repl.ts")}\n`);
});

test("dropContextFiles removes item by path", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const first = path.join(cwd, "a.txt");
  const second = path.join(cwd, "b.txt");
  const files = [first, second];
  const result = dropContextFiles(files, [first], cwd);
  assert.equal(result.ok, true);
  assert.deepEqual(result.removed, [first]);
  assert.deepEqual(files, [second]);
});

test("dropContextFiles removes item by index", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files = [
    path.join(cwd, "a.txt"),
    path.join(cwd, "b.txt"),
    path.join(cwd, "c.txt")
  ];
  const result = dropContextFiles(files, ["2"], cwd);
  assert.equal(result.ok, true);
  assert.deepEqual(result.removed, [path.join(cwd, "b.txt")]);
  assert.deepEqual(files, [path.join(cwd, "a.txt"), path.join(cwd, "c.txt")]);
});

test("dropContextFiles reports non-existent target", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files = [path.join(cwd, "a.txt")];
  const result = dropContextFiles(files, ["#3"], cwd);
  assert.equal(result.ok, false);
  assert.match(result.error ?? "", /out of range/);
  assert.deepEqual(files, [path.join(cwd, "a.txt")]);
});

test("dropContextFiles reports empty collection", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files: string[] = [];
  const result = dropContextFiles(files, ["1"], cwd);
  assert.equal(result.ok, false);
  assert.match(result.error ?? "", /empty/);
});

test("dropContextFiles supports bulk removal by mixed targets", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files = [
    path.join(cwd, "a.txt"),
    path.join(cwd, "b.txt"),
    path.join(cwd, "c.txt"),
    path.join(cwd, "d.txt")
  ];
  const result = dropContextFiles(files, ["#1", path.join(cwd, "d.txt")], cwd);
  assert.equal(result.ok, true);
  assert.deepEqual(result.removed, [path.join(cwd, "a.txt"), path.join(cwd, "d.txt")]);
  assert.deepEqual(files, [path.join(cwd, "b.txt"), path.join(cwd, "c.txt")]);
});

test("dropContextFiles keeps remaining order stable after removals", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files = [
    path.join(cwd, "a.txt"),
    path.join(cwd, "b.txt"),
    path.join(cwd, "c.txt"),
    path.join(cwd, "d.txt")
  ];
  const result = dropContextFiles(files, ["2", "4"], cwd);
  assert.equal(result.ok, true);
  assert.deepEqual(files, [path.join(cwd, "a.txt"), path.join(cwd, "c.txt")]);
});

test("buildContextSystemMessage returns paths for current context", () => {
  const cwd = path.join("C:", "workspace", "minicli");
  const files = [path.join(cwd, "src", "repl.ts"), path.join(cwd, "README.md")];
  const message = buildContextSystemMessage(files, cwd);
  assert.deepEqual(message, {
    role: "system",
    content: `Context files:\n- ${path.join("src", "repl.ts")}\n- README.md`
  });
});

test("repl /add appends file into context list output", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-repl-add-"));
  const prevCwd = process.cwd();
  try {
    const target = path.join(tmpRoot, "notes.txt");
    fs.writeFileSync(target, "notes\n", "utf8");
    process.chdir(tmpRoot);

    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH
    );
    await session.onLine("/add notes.txt");

    const output = writes.join("");
    assert.equal(errors.join(""), "");
    assert.match(output, /\[add\] added:/);
    assert.match(output, /Context files:/);
    assert.match(output, /\[1\] notes\.txt/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("repl /drop removes by index and updates context list", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), "minicli-repl-drop-"));
  const prevCwd = process.cwd();
  try {
    fs.writeFileSync(path.join(tmpRoot, "a.txt"), "a\n", "utf8");
    fs.writeFileSync(path.join(tmpRoot, "b.txt"), "b\n", "utf8");
    process.chdir(tmpRoot);

    const session = createReplSession(
      {
        stdout: (message) => writes.push(message),
        stderr: (message) => errors.push(message)
      },
      DEFAULT_MAX_INPUT_LENGTH
    );
    await session.onLine("/add a.txt");
    await session.onLine("/add b.txt");
    writes.length = 0;
    await session.onLine("/drop 1");

    const output = writes.join("");
    assert.equal(errors.join(""), "");
    assert.match(output, /\[drop\] removed 1 file\(s\)\./);
    assert.doesNotMatch(output, /\[1\] a\.txt/);
    assert.match(output, /\[1\] b\.txt/);
  } finally {
    process.chdir(prevCwd);
    fs.rmSync(tmpRoot, { recursive: true, force: true });
  }
});

test("repl session marks /exit as exit signal", async () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = await session.onLine("/exit");

  assert.equal(shouldExit, true);
  assert.equal(writes.join(""), "Bye.\n");
});

test("repl session routes unknown slash command to stderr", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: (message) => errors.push(message)
  });

  const shouldExit = await session.onLine("/nope");

  assert.equal(shouldExit, false);
  assert.equal(writes.length, 0);
  assert.match(errors.join(""), /Unknown command: \/nope/);
});

test("repl session shows completions for command prefix input", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: (message) => errors.push(message)
  });

  const shouldExit = await session.onLine("/mo");

  assert.equal(shouldExit, false);
  assert.equal(errors.length, 0);
  assert.equal(writes.join(""), "Completions:\n/model\n");
});

test("repl session preserves completion frequency ordering across restarts", async () => {
  const history: string[] = [];
  const commandHistoryRepository = {
    recordCommand: (input: { command: string; cwd: string; exitCode?: number }) => {
      history.push(input.command);
    },
    listUsageFrequency: (): Record<string, number> => {
      const result: Record<string, number> = {};
      for (const command of history) {
        result[command] = (result[command] ?? 0) + 1;
      }
      return result;
    }
  };

  const firstErrors: string[] = [];
  const firstSession = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => firstErrors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    { commandHistoryRepository }
  );

  await firstSession.onLine("/switch #1");
  await firstSession.onLine("/switch #2");
  assert.equal(history.length, 2);
  assert.match(firstErrors.join(""), /Session storage is unavailable/);

  const secondWrites: string[] = [];
  const secondErrors: string[] = [];
  const secondSession = createReplSession(
    {
      stdout: (message) => secondWrites.push(message),
      stderr: (message) => secondErrors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    { commandHistoryRepository }
  );

  await secondSession.onLine("/s");

  assert.equal(secondErrors.length, 0);
  assert.equal(secondWrites.join(""), "Completions:\n/switch\n/sessions\n");
});

test("repl session renders multiline reply content", async () => {
  const writes: string[] = [];
  const provider: LLMProvider = {
    id: "multiline",
    complete: async () => ({
      message: {
        role: "assistant",
        content: "line-1\nline-2"
      }
    })
  };
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    { provider }
  );

  await session.onLine("show lines");

  assert.equal(writes.join(""), "line-1\nline-2\n");
});

test("repl session falls back for empty provider reply", async () => {
  const writes: string[] = [];
  const provider: LLMProvider = {
    id: "empty",
    complete: async () => ({
      message: {
        role: "assistant",
        content: "   "
      }
    })
  };
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    { provider }
  );

  await session.onLine("question");

  assert.equal(writes.join(""), `${EMPTY_REPLY_PLACEHOLDER}\n`);
});

test("repl session truncates overlong provider reply", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const provider: LLMProvider = {
    id: "long",
    complete: async () => ({
      message: {
        role: "assistant",
        content: "abcdefghij"
      }
    })
  };
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      provider,
      maxReplyLength: 5
    }
  );

  await session.onLine("question");

  assert.equal(writes.join(""), "abcde...[truncated]\n");
  assert.match(errors.join(""), /reply exceeded 5 chars; truncated/);
});

test("repl /login saves key with masked output", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  let savedKey = "";
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      saveApiKey: (apiKey) => {
        savedKey = apiKey;
      }
    }
  );

  const shouldExit = await session.onLine("/login sk-12345678");

  assert.equal(shouldExit, false);
  assert.equal(savedKey, "sk-12345678");
  assert.equal(errors.length, 0);
  assert.match(writes.join(""), /API key saved: \*+5678/);
});

test("repl /login rejects empty key", async () => {
  const errors: string[] = [];
  const session = createReplSession({
    stdout: () => {},
    stderr: (message) => errors.push(message)
  });

  const shouldExit = await session.onLine("/login");

  assert.equal(shouldExit, false);
  assert.match(errors.join(""), /Usage: \/login <apiKey>/);
});

test("repl /model echoes configured model", async () => {
  const writes: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      config: {
        model: "glm-4-air",
        timeoutMs: 30000
      }
    }
  );

  const shouldExit = await session.onLine("/model");

  assert.equal(shouldExit, false);
  assert.match(writes.join(""), /Current model: glm-4-air/);
});

test("repl /model updates model and calls save callback", async () => {
  const writes: string[] = [];
  let savedModel = "";
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: () => {}
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      saveModel: (model) => {
        savedModel = model;
      }
    }
  );

  await session.onLine("/model gpt-4.1");
  await session.onLine("/model");

  assert.equal(savedModel, "gpt-4.1");
  assert.match(writes.join(""), /Model updated: gpt-4.1/);
  assert.match(writes.join(""), /Current model: gpt-4.1/);
});

test("repl /model rejects empty model name", async () => {
  const errors: string[] = [];
  const session = createReplSession({
    stdout: () => {},
    stderr: (message) => errors.push(message)
  });

  const shouldExit = await session.onLine("/model one two");

  assert.equal(shouldExit, false);
  assert.match(errors.join(""), /Usage: \/model \[name\]/);
});

test("repl command handling trims surrounding whitespace", async () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = await session.onLine("   /help   ");

  assert.equal(shouldExit, false);
  assert.equal(writes.join(""), HELP_TEXT);
});

test("repl session handles EOF close event", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  session.onClose();
  assert.match(writes.join(""), /EOF received\. Bye\./);
});

test("repl session surfaces provider errors through stderr", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const provider: LLMProvider = {
    id: "broken",
    complete: async () => {
      throw new Error("provider unavailable");
    }
  };
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      provider
    }
  );

  const shouldExit = await session.onLine("hello");

  assert.equal(shouldExit, false);
  assert.equal(writes.length, 0);
  assert.match(errors.join(""), /\[provider:error\] provider unavailable/);
});

test("repl session handles SIGINT safely", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  session.onSigint();
  assert.match(writes.join(""), /\^C/);
  assert.match(writes.join(""), /Interrupted\. Bye\./);
});

test("output buffer flushes by threshold and manual flush", () => {
  const writes: string[] = [];
  const buffer = new OutputBuffer((message) => writes.push(message), 5);

  buffer.append("ab");
  assert.equal(writes.length, 0);

  buffer.append("cde");
  assert.equal(writes.join(""), "abcde");

  buffer.append("xy");
  buffer.flush();
  assert.equal(writes.join(""), "abcdexy");
});
