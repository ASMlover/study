export interface CommandMetadata {
  readonly name: string;
  readonly description: string;
  readonly usage: string;
}

export interface CommandRegistration<TKind extends string> {
  readonly kind: TKind;
  readonly metadata: CommandMetadata;
  readonly aliases?: readonly string[];
}

export type CommandParameterType = "string" | "number" | "boolean";

export interface CommandParameterSchema {
  readonly name: string;
  readonly type: CommandParameterType;
  readonly required: boolean;
}

export interface CommandSchemaRegistration<
  TKind extends string,
  THandlerKey extends string
> extends CommandRegistration<TKind> {
  readonly acceptsArgs: boolean;
  readonly parameters: readonly CommandParameterSchema[];
  readonly examples: readonly string[];
  readonly handler: THandlerKey;
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null;
}

function readRequiredString(
  source: Record<string, unknown>,
  field: string
): string {
  const value = source[field];
  if (typeof value !== "string" || value.trim().length === 0) {
    throw new Error(`Missing required field: ${field}`);
  }
  return value;
}

function readOptionalBoolean(
  source: Record<string, unknown>,
  field: string,
  fallback: boolean
): boolean {
  const value = source[field];
  if (value === undefined) {
    return fallback;
  }
  if (typeof value !== "boolean") {
    throw new Error(`Invalid field type: ${field} must be boolean`);
  }
  return value;
}

function readOptionalStringArray(
  source: Record<string, unknown>,
  field: string
): string[] {
  const value = source[field];
  if (value === undefined) {
    return [];
  }
  if (!Array.isArray(value) || value.some((item) => typeof item !== "string")) {
    throw new Error(`Invalid field type: ${field} must be string[]`);
  }
  return value;
}

function parseExamplesField(value: unknown): string[] {
  if (value === undefined) {
    return [];
  }
  if (typeof value === "string") {
    return value
      .split(/\r?\n/)
      .map((item) => item.trim())
      .filter((item) => item.length > 0);
  }
  if (!Array.isArray(value) || value.some((item) => typeof item !== "string")) {
    throw new Error("Invalid field type: examples must be string or string[]");
  }
  return value.map((item) => item.trim()).filter((item) => item.length > 0);
}

function parseParameterSchema(value: unknown, index: number): CommandParameterSchema {
  if (!isRecord(value)) {
    throw new Error(`Invalid parameter schema at index ${index}: must be object`);
  }
  const name = readRequiredString(value, "name");
  const rawType = readRequiredString(value, "type");
  if (rawType !== "string" && rawType !== "number" && rawType !== "boolean") {
    throw new Error(
      `Invalid parameter type at index ${index}: ${rawType}; expected string|number|boolean`
    );
  }
  const required = readOptionalBoolean(value, "required", false);
  return {
    name,
    type: rawType,
    required
  };
}

export function parseCommandSchemaRegistration<
  TKind extends string,
  THandlerKey extends string
>(value: unknown): CommandSchemaRegistration<TKind, THandlerKey> {
  if (!isRecord(value)) {
    throw new Error("Command schema must be an object");
  }

  const kind = readRequiredString(value, "kind") as TKind;
  const name = readRequiredString(value, "name");
  const usage = readRequiredString(value, "usage");
  const description = readRequiredString(value, "description");
  const handler = readRequiredString(value, "handler") as THandlerKey;
  const acceptsArgs = readOptionalBoolean(value, "acceptsArgs", false);
  const aliases = readOptionalStringArray(value, "aliases");
  const examples = parseExamplesField(value.examples);
  const rawParameters = value.parameters;
  if (rawParameters !== undefined && !Array.isArray(rawParameters)) {
    throw new Error("Invalid field type: parameters must be array");
  }
  const parameters = (rawParameters ?? []).map((item, index) =>
    parseParameterSchema(item, index)
  );

  return {
    kind,
    metadata: {
      name,
      usage,
      description
    },
    acceptsArgs,
    aliases,
    parameters,
    examples,
    handler
  };
}

export function parseCommandSchemaRegistrations<
  TKind extends string,
  THandlerKey extends string
>(value: unknown): CommandSchemaRegistration<TKind, THandlerKey>[] {
  if (!Array.isArray(value)) {
    throw new Error("Command schema collection must be an array");
  }
  return value.map((item) => parseCommandSchemaRegistration<TKind, THandlerKey>(item));
}

export function registerCommandSchemas<
  TKind extends string,
  THandlerKey extends string
>(
  registry: CommandRegistry<TKind>,
  schemaCollection: unknown,
  availableHandlers: ReadonlySet<THandlerKey>
): CommandSchemaRegistration<TKind, THandlerKey>[] {
  const parsed = parseCommandSchemaRegistrations<TKind, THandlerKey>(
    schemaCollection
  );
  for (const command of parsed) {
    if (!availableHandlers.has(command.handler)) {
      throw new Error(`Missing command handler: ${command.handler}`);
    }
    registry.register(command);
  }
  return parsed;
}

export class CommandRegistry<TKind extends string> {
  private readonly byToken = new Map<string, CommandRegistration<TKind>>();
  private readonly byKind = new Map<TKind, CommandRegistration<TKind>>();
  private readonly ordered: CommandRegistration<TKind>[] = [];

  register(command: CommandRegistration<TKind>): void {
    this.ensureDistinctKind(command);
    this.ensureDistinctTokens(command);

    this.byKind.set(command.kind, command);
    this.byToken.set(command.metadata.name, command);
    for (const alias of command.aliases ?? []) {
      this.byToken.set(alias, command);
    }
    this.ordered.push(command);
  }

  registerMany(commands: readonly CommandRegistration<TKind>[]): void {
    for (const command of commands) {
      this.register(command);
    }
  }

  resolve(token: string): CommandRegistration<TKind> | undefined {
    return this.byToken.get(token);
  }

  get(kind: TKind): CommandRegistration<TKind> | undefined {
    return this.byKind.get(kind);
  }

  list(): readonly CommandRegistration<TKind>[] {
    return [...this.ordered];
  }

  private ensureDistinctKind(command: CommandRegistration<TKind>): void {
    if (this.byKind.has(command.kind)) {
      throw new Error(`Duplicate command kind: ${command.kind}`);
    }
  }

  private ensureDistinctTokens(command: CommandRegistration<TKind>): void {
    const nextTokens = [command.metadata.name, ...(command.aliases ?? [])];
    for (const token of nextTokens) {
      const existing = this.byToken.get(token);
      if (existing) {
        throw new Error(
          `Duplicate command token: ${token} already used by ${existing.kind}`
        );
      }
    }
  }
}
