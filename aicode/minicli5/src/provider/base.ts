import type { ChatRequest, ChatResponse, StreamChunk, Provider } from "../types.js";

export type { Provider };

export abstract class BaseProvider implements Provider {
  abstract chat(req: ChatRequest): Promise<ChatResponse>;
  abstract chatStream(req: ChatRequest): AsyncIterable<StreamChunk>;
}
