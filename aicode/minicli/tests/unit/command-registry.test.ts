import test from "node:test";
import assert from "node:assert/strict";
import { CommandRegistry } from "../../src/command-registry";

type TestCommandKind = "help" | "exit" | "model";

test("command registry supports command registration and lookup by token", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.register({
    kind: "help",
    metadata: {
      name: "/help",
      usage: "/help",
      description: "show help"
    }
  });

  const command = registry.resolve("/help");
  assert.equal(command?.kind, "help");
  assert.equal(command?.metadata.usage, "/help");
});

test("command registry resolves aliases to the same command", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.register({
    kind: "exit",
    metadata: {
      name: "/exit",
      usage: "/exit",
      description: "exit repl"
    },
    aliases: ["/quit"]
  });

  assert.equal(registry.resolve("/exit")?.kind, "exit");
  assert.equal(registry.resolve("/quit")?.kind, "exit");
});

test("command registry rejects duplicate command tokens", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.register({
    kind: "help",
    metadata: {
      name: "/help",
      usage: "/help",
      description: "show help"
    }
  });

  assert.throws(
    () =>
      registry.register({
        kind: "model",
        metadata: {
          name: "/help",
          usage: "/model",
          description: "set model"
        }
      }),
    /Duplicate command token/
  );
});

test("command registry returns undefined for unknown commands", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.register({
    kind: "help",
    metadata: {
      name: "/help",
      usage: "/help",
      description: "show help"
    }
  });

  assert.equal(registry.resolve("/missing"), undefined);
});

test("command registry supports bulk registration", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.registerMany([
    {
      kind: "help",
      metadata: {
        name: "/help",
        usage: "/help",
        description: "show help"
      }
    },
    {
      kind: "model",
      metadata: {
        name: "/model",
        usage: "/model [name]",
        description: "show model"
      }
    }
  ]);

  assert.equal(registry.resolve("/help")?.kind, "help");
  assert.equal(registry.resolve("/model")?.kind, "model");
});

test("command registry keeps registration order stable", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  registry.registerMany([
    {
      kind: "help",
      metadata: {
        name: "/help",
        usage: "/help",
        description: "show help"
      }
    },
    {
      kind: "exit",
      metadata: {
        name: "/exit",
        usage: "/exit",
        description: "exit repl"
      },
      aliases: ["/quit"]
    },
    {
      kind: "model",
      metadata: {
        name: "/model",
        usage: "/model [name]",
        description: "show model"
      }
    }
  ]);

  assert.deepEqual(
    registry.list().map((item) => item.metadata.name),
    ["/help", "/exit", "/model"]
  );
});
