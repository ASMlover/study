import test from "node:test";
import assert from "node:assert/strict";
import {
  buildDefaultSessionTitle,
  classifyReplInput,
  createRuntimeProvider,
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  EMPTY_REPLY_PLACEHOLDER,
  formatSessionTimestamp,
  HELP_TEXT,
  matchReplCommand,
  OutputBuffer,
  parseNewSessionTitle,
  parseReplLine,
  renderAssistantReply,
  resolveUniqueSessionTitle
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
});

test("parseNewSessionTitle joins args and trims spaces", () => {
  assert.equal(parseNewSessionTitle(["my", "chat"]), "my chat");
  assert.equal(parseNewSessionTitle([]), undefined);
  assert.equal(parseNewSessionTitle(["   "]), undefined);
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
        }
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
