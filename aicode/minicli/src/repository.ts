import { DatabaseConnection, withTransaction } from "./db";

export type MessageRole = "system" | "user" | "assistant" | "tool";

export interface SessionRecord {
  id: number;
  title: string;
  createdAt: string;
  updatedAt: string;
}

export interface MessageRecord {
  id: number;
  sessionId: number;
  role: MessageRole;
  content: string;
  createdAt: string;
}

export interface PaginationParams {
  limit?: number;
  offset?: number;
}

export type RunAuditRiskLevel = "low" | "medium" | "high";
export type RunAuditApprovalStatus =
  | "not_required"
  | "approved"
  | "rejected"
  | "timeout";

export interface RunAuditRecord {
  id: number;
  command: string;
  riskLevel: RunAuditRiskLevel;
  approvalStatus: RunAuditApprovalStatus;
  executed: boolean;
  exitCode: number | null;
  stdout: string;
  stderr: string;
  createdAt: string;
}

function normalizePagination(params?: PaginationParams): {
  limit?: number;
  offset: number;
} {
  const offset =
    typeof params?.offset === "number" && params.offset > 0
      ? Math.floor(params.offset)
      : 0;

  if (params?.limit === undefined) {
    return { offset };
  }

  if (typeof params.limit !== "number" || params.limit <= 0) {
    return { limit: 0, offset };
  }

  return {
    limit: Math.floor(params.limit),
    offset
  };
}

function mapSession(row: unknown): SessionRecord {
  const typed = row as {
    id: number;
    title: string;
    created_at: string;
    updated_at: string;
  };
  return {
    id: typed.id,
    title: typed.title,
    createdAt: typed.created_at,
    updatedAt: typed.updated_at
  };
}

function mapMessage(row: unknown): MessageRecord {
  const typed = row as {
    id: number;
    session_id: number;
    role: MessageRole;
    content: string;
    created_at: string;
  };
  return {
    id: typed.id,
    sessionId: typed.session_id,
    role: typed.role,
    content: typed.content,
    createdAt: typed.created_at
  };
}

function mapRunAudit(row: unknown): RunAuditRecord {
  const typed = row as {
    id: number;
    command: string;
    risk_level: RunAuditRiskLevel;
    approval_status: RunAuditApprovalStatus;
    executed: number;
    exit_code: number | null;
    stdout: string;
    stderr: string;
    created_at: string;
  };
  return {
    id: typed.id,
    command: typed.command,
    riskLevel: typed.risk_level,
    approvalStatus: typed.approval_status,
    executed: typed.executed === 1,
    exitCode: typed.exit_code,
    stdout: typed.stdout,
    stderr: typed.stderr,
    createdAt: typed.created_at
  };
}

export class SessionRepository {
  constructor(private readonly connection: DatabaseConnection) {}

  createSession(title?: string): SessionRecord {
    const normalized = title?.trim() ? title.trim() : "New Session";
    this.connection
      .prepare("INSERT INTO sessions(title) VALUES (?)")
      .run(normalized);
    const created = this.connection
      .prepare(
        "SELECT id, title, created_at, updated_at FROM sessions WHERE id = last_insert_rowid()"
      )
      .get();

    return mapSession(created);
  }

  listSessions(params?: PaginationParams): SessionRecord[] {
    const pagination = normalizePagination(params);
    if (pagination.limit === 0) {
      return [];
    }

    if (pagination.limit === undefined) {
      const rows = this.connection
        .prepare(
          "SELECT id, title, created_at, updated_at FROM sessions ORDER BY updated_at DESC, id DESC"
        )
        .all();
      return rows.map(mapSession);
    }

    const rows = this.connection
      .prepare(
        "SELECT id, title, created_at, updated_at FROM sessions ORDER BY updated_at DESC, id DESC LIMIT ? OFFSET ?"
      )
      .all(pagination.limit, pagination.offset);
    return rows.map(mapSession);
  }
}

export class MessageRepository {
  constructor(private readonly connection: DatabaseConnection) {}

  createMessage(input: {
    sessionId: number;
    role: MessageRole;
    content: string;
  }): MessageRecord {
    const content = input.content.trim();
    if (content.length === 0) {
      throw new Error("Message content cannot be empty.");
    }

    this.connection
      .prepare("INSERT INTO messages(session_id, role, content) VALUES (?, ?, ?)")
      .run(input.sessionId, input.role, content);
    this.connection
      .prepare("UPDATE sessions SET updated_at = datetime('now') WHERE id = ?")
      .run(input.sessionId);
    const created = this.connection
      .prepare(
        "SELECT id, session_id, role, content, created_at FROM messages WHERE id = last_insert_rowid()"
      )
      .get();

    return mapMessage(created);
  }

  createMessagesInTransaction(
    sessionId: number,
    messages: Array<{
      role: MessageRole;
      content: string;
    }>
  ): MessageRecord[] {
    return withTransaction(this.connection, () => {
      const created: MessageRecord[] = [];
      for (const item of messages) {
        created.push(
          this.createMessage({
            sessionId,
            role: item.role,
            content: item.content
          })
        );
      }
      return created;
    });
  }

  listMessagesBySession(
    sessionId: number,
    params?: PaginationParams
  ): MessageRecord[] {
    const pagination = normalizePagination(params);
    if (pagination.limit === 0) {
      return [];
    }

    if (pagination.limit === undefined) {
      const rows = this.connection
        .prepare(
          "SELECT id, session_id, role, content, created_at FROM messages WHERE session_id = ? ORDER BY created_at ASC, id ASC"
        )
        .all(sessionId);
      return rows.map(mapMessage);
    }

    const rows = this.connection
      .prepare(
        "SELECT id, session_id, role, content, created_at FROM messages WHERE session_id = ? ORDER BY created_at ASC, id ASC LIMIT ? OFFSET ?"
      )
      .all(sessionId, pagination.limit, pagination.offset);
    return rows.map(mapMessage);
  }
}

export class CommandHistoryRepository {
  constructor(private readonly connection: DatabaseConnection) {}

  recordCommand(input: { command: string; cwd: string; exitCode?: number }): void {
    const command = input.command.trim();
    if (command.length === 0) {
      return;
    }

    this.connection
      .prepare(
        "INSERT INTO command_history(command, cwd, exit_code) VALUES (?, ?, ?)"
      )
      .run(command, input.cwd, input.exitCode ?? null);
  }

  listUsageFrequency(): Record<string, number> {
    const rows = this.connection
      .prepare(
        "SELECT command, COUNT(*) AS usage_count FROM command_history GROUP BY command"
      )
      .all();

    const frequencies: Record<string, number> = {};
    for (const row of rows) {
      const typed = row as {
        command: string;
        usage_count: number;
      };
      frequencies[typed.command] = typed.usage_count;
    }
    return frequencies;
  }
}

export class RunAuditRepository {
  constructor(private readonly connection: DatabaseConnection) {}

  recordAudit(input: {
    command: string;
    riskLevel: RunAuditRiskLevel;
    approvalStatus: RunAuditApprovalStatus;
    executed: boolean;
    exitCode?: number | null;
    stdout?: string;
    stderr?: string;
  }): RunAuditRecord {
    const command = input.command.trim();
    const normalizedCommand = command.length > 0 ? command : "(empty)";
    const stdout = input.stdout ?? "";
    const stderr = input.stderr ?? "";
    const executed = input.executed ? 1 : 0;
    this.connection
      .prepare(
        "INSERT INTO run_audit(command, risk_level, approval_status, executed, exit_code, stdout, stderr) VALUES (?, ?, ?, ?, ?, ?, ?)"
      )
      .run(
        normalizedCommand,
        input.riskLevel,
        input.approvalStatus,
        executed,
        input.exitCode ?? null,
        stdout,
        stderr
      );

    const created = this.connection
      .prepare(
        "SELECT id, command, risk_level, approval_status, executed, exit_code, stdout, stderr, created_at FROM run_audit WHERE id = last_insert_rowid()"
      )
      .get();
    return mapRunAudit(created);
  }

  listAudits(params?: {
    limit?: number;
    offset?: number;
    approvalStatus?: RunAuditApprovalStatus;
  }): RunAuditRecord[] {
    const pagination = normalizePagination(params);
    if (pagination.limit === 0) {
      return [];
    }

    const hasStatusFilter = params?.approvalStatus !== undefined;
    if (pagination.limit === undefined) {
      const rows = hasStatusFilter
        ? this.connection
            .prepare(
              "SELECT id, command, risk_level, approval_status, executed, exit_code, stdout, stderr, created_at FROM run_audit WHERE approval_status = ? ORDER BY created_at DESC, id DESC"
            )
            .all(params?.approvalStatus)
        : this.connection
            .prepare(
              "SELECT id, command, risk_level, approval_status, executed, exit_code, stdout, stderr, created_at FROM run_audit ORDER BY created_at DESC, id DESC"
            )
            .all();
      return rows.map(mapRunAudit);
    }

    const rows = hasStatusFilter
      ? this.connection
          .prepare(
            "SELECT id, command, risk_level, approval_status, executed, exit_code, stdout, stderr, created_at FROM run_audit WHERE approval_status = ? ORDER BY created_at DESC, id DESC LIMIT ? OFFSET ?"
          )
          .all(params?.approvalStatus, pagination.limit, pagination.offset)
      : this.connection
          .prepare(
            "SELECT id, command, risk_level, approval_status, executed, exit_code, stdout, stderr, created_at FROM run_audit ORDER BY created_at DESC, id DESC LIMIT ? OFFSET ?"
          )
          .all(pagination.limit, pagination.offset);
    return rows.map(mapRunAudit);
  }
}

export function createRepositories(connection: DatabaseConnection): {
  sessions: SessionRepository;
  messages: MessageRepository;
  commandHistory: CommandHistoryRepository;
  runAudit: RunAuditRepository;
} {
  return {
    sessions: new SessionRepository(connection),
    messages: new MessageRepository(connection),
    commandHistory: new CommandHistoryRepository(connection),
    runAudit: new RunAuditRepository(connection)
  };
}
