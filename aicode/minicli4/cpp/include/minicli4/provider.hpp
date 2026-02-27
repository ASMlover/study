#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "minicli4/config.hpp"

namespace minicli4 {

struct ToolCall {
  std::string id;
  std::string name;
  std::string arguments_json;
};

struct ChatChoice {
  std::string message_content;
  std::string delta_content;
  std::vector<ToolCall> tool_calls;
};

struct ChatResponse {
  std::vector<ChatChoice> choices;
};

struct ProviderRequest {
  std::vector<std::pair<std::string, std::string>> messages;
  bool stream{false};
  double temperature{0.7};
  int max_tokens{1024};
};

struct ProviderReply {
  bool streamed{false};
  ChatResponse response;
  std::vector<ChatResponse> stream_chunks;
};

class ProviderError : public std::runtime_error {
 public:
  ProviderError(const std::string& message, bool retryable);
  bool retryable() const;

 private:
  bool retryable_;
};

class IProvider {
 public:
  virtual ~IProvider() = default;
  virtual ProviderReply chat(const ProviderRequest& req) = 0;
};

class GLMProvider : public IProvider {
 public:
  explicit GLMProvider(RuntimeConfig config);
  ProviderReply chat(const ProviderRequest& req) override;
  static std::vector<ChatResponse> parse_sse_lines(const std::vector<std::string>& lines);

 private:
  RuntimeConfig config_;
};

}  // namespace minicli4
