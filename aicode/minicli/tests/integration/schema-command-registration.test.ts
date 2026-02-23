import test from "node:test";
import assert from "node:assert/strict";
import {
  CommandRegistry,
  registerCommandSchemas
} from "../../src/command-registry";
import {
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  KnownReplCommandKind
} from "../../src/repl";

test("repl executes command registered through schema", async () => {
  const registry = new CommandRegistry<KnownReplCommandKind>();
  registerCommandSchemas<KnownReplCommandKind, KnownReplCommandKind>(
    registry,
    [
      {
        kind: "help",
        name: "/help",
        usage: "/help",
        description: "show help",
        acceptsArgs: false,
        handler: "help"
      },
      {
        kind: "exit",
        name: "/exit",
        usage: "/exit",
        description: "exit",
        acceptsArgs: false,
        handler: "exit",
        aliases: ["/quit"]
      }
    ],
    new Set<KnownReplCommandKind>(["help", "exit"])
  );

  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      commandRegistry: registry
    }
  );

  const shouldExit = await session.onLine("/quit");

  assert.equal(shouldExit, true);
  assert.equal(errors.join(""), "");
  assert.equal(writes.join(""), "Bye.\n");
});
