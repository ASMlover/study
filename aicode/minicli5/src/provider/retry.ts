import { log } from "../utils/logger.js";

export interface RetryOptions {
  maxRetries: number;
  baseDelay?: number;   // ms, default 1000
  maxDelay?: number;    // ms, default 30000
}

const RETRYABLE_STATUSES = new Set([429, 500, 502, 503, 504]);

export function isRetryable(status: number): boolean {
  return RETRYABLE_STATUSES.has(status);
}

export async function withRetry<T>(
  fn: () => Promise<T>,
  opts: RetryOptions,
): Promise<T> {
  const { maxRetries, baseDelay = 1000, maxDelay = 30000 } = opts;
  let lastError: unknown;

  for (let attempt = 0; attempt <= maxRetries; attempt++) {
    try {
      return await fn();
    } catch (err: unknown) {
      lastError = err;
      const status = (err as { status?: number }).status;

      if (attempt >= maxRetries || (status && !isRetryable(status))) {
        throw err;
      }

      const delay = Math.min(baseDelay * Math.pow(2, attempt), maxDelay);
      const jitter = delay * (0.5 + Math.random() * 0.5);
      log("warn", `Retry attempt ${attempt + 1}/${maxRetries}`, { status, delay: jitter });
      await new Promise(r => setTimeout(r, jitter));
    }
  }

  throw lastError;
}
