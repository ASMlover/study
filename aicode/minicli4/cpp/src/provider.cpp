#include "minicli4/provider.hpp"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#define MINICLI4_POPEN _popen
#define MINICLI4_PCLOSE _pclose
#else
#define MINICLI4_POPEN popen
#define MINICLI4_PCLOSE pclose
#endif

namespace {
std::string json_escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 16);
  for (const char c : s) {
    switch (c) {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out.push_back(c);
        break;
    }
  }
  return out;
}

std::string shell_escape_double_quotes(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 16);
  for (const char c : s) {
    if (c == '"') {
      out += "\\\"";
    } else {
      out.push_back(c);
    }
  }
  return out;
}

std::string json_unescape(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    const char c = s[i];
    if (c == '\\' && i + 1 < s.size()) {
      const char n = s[i + 1];
      if (n == 'n') {
        out.push_back('\n');
        i++;
        continue;
      }
      if (n == 'r') {
        out.push_back('\r');
        i++;
        continue;
      }
      if (n == 't') {
        out.push_back('\t');
        i++;
        continue;
      }
      if (n == '\\' || n == '"') {
        out.push_back(n);
        i++;
        continue;
      }
    }
    out.push_back(c);
  }
  return out;
}

std::string extract_json_string_after(const std::string& text, const std::string& marker, size_t search_from = 0) {
  const auto pos = text.find(marker, search_from);
  if (pos == std::string::npos) {
    return {};
  }
  size_t i = pos + marker.size();
  std::string raw;
  bool escaped = false;
  while (i < text.size()) {
    const char c = text[i];
    if (!escaped && c == '"') {
      return json_unescape(raw);
    }
    if (!escaped && c == '\\') {
      escaped = true;
      raw.push_back(c);
      i++;
      continue;
    }
    escaped = false;
    raw.push_back(c);
    i++;
  }
  return {};
}

std::string build_messages_json(const std::vector<std::pair<std::string, std::string>>& messages) {
  std::ostringstream out;
  out << "[";
  for (size_t i = 0; i < messages.size(); i++) {
    if (i > 0) {
      out << ",";
    }
    out << "{\"role\":\"" << json_escape(messages[i].first) << "\",\"content\":\"" << json_escape(messages[i].second) << "\"}";
  }
  out << "]";
  return out.str();
}

std::string run_command_capture_all(const std::string& command) {
  FILE* pipe = MINICLI4_POPEN(command.c_str(), "r");
  if (!pipe) {
    throw std::runtime_error("failed to start curl process");
  }
  std::string output;
  char buffer[4096];
  while (fgets(buffer, static_cast<int>(sizeof(buffer)), pipe) != nullptr) {
    output.append(buffer);
  }
  const int code = MINICLI4_PCLOSE(pipe);
  if (code != 0) {
    throw std::runtime_error("curl request failed: " + output);
  }
  return output;
}
}  // namespace

namespace minicli4 {

ProviderError::ProviderError(const std::string& message, bool retryable)
    : std::runtime_error(message), retryable_(retryable) {}

bool ProviderError::retryable() const {
  return retryable_;
}

GLMProvider::GLMProvider(RuntimeConfig config) : config_(std::move(config)) {
  if (config_.model != "glm-5") {
    throw ProviderError("model must be glm-5", false);
  }
}

ProviderReply GLMProvider::chat(const ProviderRequest& req) {
  if (config_.api_key.empty()) {
    throw ProviderError("missing api_key for glm-5 provider", false);
  }

  const std::string url = config_.base_url + "/chat/completions";
  const std::string payload =
      std::string("{\"model\":\"glm-5\",\"messages\":") + build_messages_json(req.messages) +
      ",\"stream\":" + (req.stream ? "true" : "false") +
      ",\"temperature\":" + std::to_string(req.temperature) +
      ",\"max_tokens\":" + std::to_string(req.max_tokens) + "}";

  std::string curl_cmd =
      std::string("curl -sS ") + (req.stream ? "-N " : "") +
      "-X POST \"" + shell_escape_double_quotes(url) +
      "\" -H \"Content-Type: application/json\" -H \"Authorization: Bearer " + shell_escape_double_quotes(config_.api_key) +
      "\" --data \"" + shell_escape_double_quotes(payload) + "\" 2>&1";

  if (req.stream) {
    const std::string raw = run_command_capture_all(curl_cmd);
    std::istringstream in(raw);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }
      lines.push_back(line);
    }

    ProviderReply reply;
    reply.streamed = true;
    reply.stream_chunks = parse_sse_lines(lines);
    if (reply.stream_chunks.empty()) {
      const std::string fallback = extract_json_string_after(raw, "\"content\":\"");
      if (!fallback.empty()) {
        reply.stream_chunks.push_back(ChatResponse{{ChatChoice{"", fallback, {}}}});
      }
    }
    return reply;
  }

  const std::string raw = run_command_capture_all(curl_cmd);
  const std::string content = extract_json_string_after(raw, "\"content\":\"");
  ProviderReply reply;
  reply.streamed = false;
  reply.response.choices.push_back(ChatChoice{content.empty() ? "(empty model response)" : content, "", {}});
  return reply;
}

std::vector<ChatResponse> GLMProvider::parse_sse_lines(const std::vector<std::string>& lines) {
  std::vector<ChatResponse> out;
  for (const auto& line : lines) {
    if (line.rfind("data: ", 0) != 0) {
      continue;
    }
    const auto payload = line.substr(6);
    if (payload == "[DONE]") {
      break;
    }

    std::string content = extract_json_string_after(payload, "\"content\":\"");
    if (content.empty()) {
      content = extract_json_string_after(payload, "\"reasoning_content\":\"");
    }
    if (content.empty()) {
      continue;
    }
    out.push_back(ChatResponse{{ChatChoice{"", content, {}}}});
  }
  return out;
}

}  // namespace minicli4
