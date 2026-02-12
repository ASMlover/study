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
