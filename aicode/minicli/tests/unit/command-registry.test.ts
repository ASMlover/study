import test from "node:test";
import assert from "node:assert/strict";
import {
  CommandRegistry,
  parseCommandSchemaRegistration,
  registerCommandSchemas
} from "../../src/command-registry";

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

test("command schema registration passes and registers command", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  const parsed = registerCommandSchemas<TestCommandKind, "help">(
    registry,
    [
      {
        kind: "help",
        name: "/help",
        usage: "/help",
        description: "show help",
        acceptsArgs: false,
        handler: "help",
        aliases: ["/h"],
        parameters: [],
        examples: ["/help"]
      }
    ],
    new Set<"help">(["help"])
  );

  assert.equal(parsed.length, 1);
  assert.equal(parsed[0].kind, "help");
  assert.equal(parsed[0].handler, "help");
  assert.deepEqual(parsed[0].examples, ["/help"]);
  assert.equal(registry.resolve("/h")?.kind, "help");
});

test("command schema registration throws when required field is missing", () => {
  assert.throws(
    () =>
      parseCommandSchemaRegistration({
        kind: "help",
        name: "/help",
        usage: "/help",
        acceptsArgs: false,
        handler: "help"
      }),
    /Missing required field: description/
  );
});

test("command schema registration validates parameter types", () => {
  const parsed = parseCommandSchemaRegistration({
    kind: "model",
    name: "/model",
    usage: "/model [name]",
    description: "show model",
    acceptsArgs: true,
    handler: "model",
    parameters: [
      {
        name: "name",
        type: "string",
        required: false
      }
    ]
  });
  assert.deepEqual(parsed.parameters, [
    {
      name: "name",
      type: "string",
      required: false
    }
  ]);

  assert.throws(
    () =>
      parseCommandSchemaRegistration({
        kind: "model",
        name: "/model",
        usage: "/model [name]",
        description: "show model",
        acceptsArgs: true,
        handler: "model",
        parameters: [
          {
            name: "name",
            type: "integer"
          }
        ]
      }),
    /Invalid parameter type/
  );
});

test("command schema registration rejects missing handler mapping", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  assert.throws(
    () =>
      registerCommandSchemas<TestCommandKind, "help" | "model">(
        registry,
        [
          {
            kind: "model",
            name: "/model",
            usage: "/model [name]",
            description: "show model",
            acceptsArgs: true,
            handler: "model"
          }
        ],
        new Set<"help" | "model">(["help"])
      ),
    /Missing command handler: model/
  );
});

test("command schema registration rejects alias conflicts", () => {
  const registry = new CommandRegistry<TestCommandKind>();
  assert.throws(
    () =>
      registerCommandSchemas<TestCommandKind, TestCommandKind>(
        registry,
        [
          {
            kind: "help",
            name: "/help",
            usage: "/help",
            description: "show help",
            acceptsArgs: false,
            handler: "help",
            aliases: ["/shared"]
          },
          {
            kind: "exit",
            name: "/exit",
            usage: "/exit",
            description: "exit",
            acceptsArgs: false,
            handler: "exit",
            aliases: ["/shared"]
          }
        ],
        new Set<TestCommandKind>(["help", "exit", "model"])
      ),
    /Duplicate command token: \/shared/
  );
});

test("command schema registration parses examples from multiline string", () => {
  const parsed = parseCommandSchemaRegistration({
    kind: "help",
    name: "/help",
    usage: "/help",
    description: "show help",
    acceptsArgs: false,
    handler: "help",
    examples: "/help\n/help --verbose\n\n"
  });

  assert.deepEqual(parsed.examples, ["/help", "/help --verbose"]);
});
