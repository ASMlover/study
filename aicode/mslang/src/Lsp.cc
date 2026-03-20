// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <cstdio>
#include <iostream>
#include "Compiler.hh"
#include "Lsp.hh"
#include "Scanner.hh"

#if defined(_WIN32)
# include <fcntl.h>
# include <io.h>
#endif

namespace ms {

// =============================================================================
// Transport layer
// =============================================================================

str_t LspServer::read_message() noexcept {
  // Read Content-Length header
  str_t header;
  int content_length = 0;

  for (;;) {
    str_t line;
    if (!std::getline(std::cin, line)) {
      running_ = false;
      return "";
    }
    // Strip trailing \r if present
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty()) break; // blank line separates header from body

    if (line.starts_with("Content-Length: ")) {
      content_length = std::stoi(line.substr(16));
    }
  }

  if (content_length <= 0) return "";

  str_t body(static_cast<sz_t>(content_length), '\0');
  std::cin.read(body.data(), content_length);
  return body;
}

void LspServer::send_message(const json::Value& msg) noexcept {
  str_t body = msg.serialize();
  std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
  std::cout.flush();
}

void LspServer::send_response(const json::Value& id, json::Value result) noexcept {
  auto msg = json::Value::object();
  msg["jsonrpc"] = json::Value("2.0");
  msg["id"] = id;
  msg["result"] = std::move(result);
  send_message(msg);
}

void LspServer::send_error(const json::Value& id, int code, strv_t message) noexcept {
  auto msg = json::Value::object();
  msg["jsonrpc"] = json::Value("2.0");
  msg["id"] = id;
  auto err = json::Value::object();
  err["code"] = json::Value(code);
  err["message"] = json::Value(str_t(message));
  msg["error"] = std::move(err);
  send_message(msg);
}

void LspServer::send_notification(strv_t method, json::Value params) noexcept {
  auto msg = json::Value::object();
  msg["jsonrpc"] = json::Value("2.0");
  msg["method"] = json::Value(str_t(method));
  msg["params"] = std::move(params);
  send_message(msg);
}

// =============================================================================
// Main loop & dispatch
// =============================================================================

void LspServer::run() noexcept {
#if defined(_WIN32)
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
#endif

  while (running_) {
    str_t body = read_message();
    if (body.empty()) continue;

    auto msg = json::Value::parse(body);
    if (msg.is_object()) dispatch(msg);
  }
}

void LspServer::dispatch(const json::Value& msg) noexcept {
  if (!msg.has("method")) return;
  auto method = msg["method"].as_string();
  auto id = msg.has("id") ? msg["id"] : json::Value{};
  auto params = msg.has("params") ? msg["params"] : json::Value::object();

  if (method == "initialize")                         handle_initialize(id, params);
  else if (method == "initialized")                   handle_initialized();
  else if (method == "shutdown")                       handle_shutdown(id);
  else if (method == "exit")                           handle_exit();
  else if (method == "textDocument/didOpen")           handle_did_open(params);
  else if (method == "textDocument/didChange")         handle_did_change(params);
  else if (method == "textDocument/didClose")          handle_did_close(params);
  else if (method == "textDocument/semanticTokens/full") handle_semantic_tokens_full(id, params);
  else if (method == "textDocument/documentSymbol")    handle_document_symbol(id, params);
  else if (!id.is_null()) {
    // Unknown request — return MethodNotFound
    send_error(id, -32601, "Method not found");
  }
}

// =============================================================================
// Lifecycle handlers
// =============================================================================

void LspServer::handle_initialize(const json::Value& id, [[maybe_unused]] const json::Value& params) noexcept {
  // Semantic token legend
  auto token_types = json::Value::array();
  token_types.push(json::Value("keyword"));    // 0
  token_types.push(json::Value("string"));     // 1
  token_types.push(json::Value("number"));     // 2
  token_types.push(json::Value("variable"));   // 3
  token_types.push(json::Value("function"));   // 4
  token_types.push(json::Value("class"));      // 5
  token_types.push(json::Value("comment"));    // 6
  token_types.push(json::Value("operator"));   // 7

  auto token_modifiers = json::Value::array();
  token_modifiers.push(json::Value("declaration"));  // 0
  token_modifiers.push(json::Value("definition"));   // 1

  auto legend = json::Value::object();
  legend["tokenTypes"] = std::move(token_types);
  legend["tokenModifiers"] = std::move(token_modifiers);

  auto semantic_tokens = json::Value::object();
  semantic_tokens["legend"] = std::move(legend);
  semantic_tokens["full"] = json::Value(true);

  auto capabilities = json::Value::object();
  capabilities["textDocumentSync"] = json::Value(1); // Full sync
  capabilities["semanticTokensProvider"] = std::move(semantic_tokens);
  capabilities["documentSymbolProvider"] = json::Value(true);

  auto result = json::Value::object();
  result["capabilities"] = std::move(capabilities);

  auto server_info = json::Value::object();
  server_info["name"] = json::Value("maple-lsp");
  server_info["version"] = json::Value("1.0.0");
  result["serverInfo"] = std::move(server_info);

  initialized_ = true;
  send_response(id, std::move(result));
}

void LspServer::handle_initialized() noexcept {
  // Client acknowledged initialization — nothing to do
}

void LspServer::handle_shutdown(const json::Value& id) noexcept {
  send_response(id, json::Value{});
}

void LspServer::handle_exit() noexcept {
  running_ = false;
}

// =============================================================================
// Document management
// =============================================================================

void LspServer::handle_did_open(const json::Value& params) noexcept {
  auto& doc = params["textDocument"];
  auto uri = doc["uri"].as_string();
  auto text = doc["text"].as_string();
  documents_[uri] = text;
  publish_diagnostics(uri);
}

void LspServer::handle_did_change(const json::Value& params) noexcept {
  auto& doc = params["textDocument"];
  auto uri = doc["uri"].as_string();
  auto& changes = params["contentChanges"];
  if (changes.is_array() && changes.size() > 0) {
    documents_[uri] = changes.at(0)["text"].as_string();
  }
  publish_diagnostics(uri);
}

void LspServer::handle_did_close(const json::Value& params) noexcept {
  auto uri = params["textDocument"]["uri"].as_string();
  documents_.erase(uri);

  // Clear diagnostics for closed file
  auto diag_params = json::Value::object();
  diag_params["uri"] = json::Value(uri);
  diag_params["diagnostics"] = json::Value::array();
  send_notification("textDocument/publishDiagnostics", std::move(diag_params));
}

// =============================================================================
// Diagnostics
// =============================================================================

str_t LspServer::uri_to_path(const str_t& uri) noexcept {
  // Convert file:///path/to/file or file:///C%3A/... to local path
  str_t path = uri;
  if (path.starts_with("file:///")) {
    path = path.substr(8);
    // URL decode %xx sequences
    str_t decoded;
    for (sz_t i = 0; i < path.size(); ++i) {
      if (path[i] == '%' && i + 2 < path.size()) {
        auto hex = path.substr(i + 1, 2);
        decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
        i += 2;
      } else if (path[i] == '/') {
        decoded += '/';
      } else {
        decoded += path[i];
      }
    }
    path = decoded;
  }
  return path;
}

void LspServer::publish_diagnostics(const str_t& uri) noexcept {
  auto it = documents_.find(uri);
  if (it == documents_.end()) return;

  auto& source = it->second;
  auto path = uri_to_path(uri);

  std::vector<Diagnostic> diags;
  compile(source, path, diags);

  auto lsp_diags = json::Value::array();
  for (auto& d : diags) {
    auto diag = json::Value::object();
    auto range = json::Value::object();
    auto start = json::Value::object();
    auto end = json::Value::object();

    // LSP uses 0-based line/column; Maple uses 1-based
    start["line"] = json::Value(d.line - 1);
    start["character"] = json::Value(d.column > 0 ? d.column - 1 : 0);
    end["line"] = json::Value(d.line - 1);
    end["character"] = json::Value(d.end_column > 0 ? d.end_column - 1 : d.column);

    range["start"] = std::move(start);
    range["end"] = std::move(end);
    diag["range"] = std::move(range);
    diag["severity"] = json::Value(1); // Error
    diag["source"] = json::Value("maple");
    diag["message"] = json::Value(d.message);
    lsp_diags.push(std::move(diag));
  }

  auto params = json::Value::object();
  params["uri"] = json::Value(uri);
  params["diagnostics"] = std::move(lsp_diags);
  send_notification("textDocument/publishDiagnostics", std::move(params));
}

// =============================================================================
// Semantic tokens
// =============================================================================

// Semantic token type indices (must match legend in handle_initialize)
static constexpr int kSEM_KEYWORD  = 0;
static constexpr int kSEM_STRING   = 1;
static constexpr int kSEM_NUMBER   = 2;
static constexpr int kSEM_VARIABLE = 3;
static constexpr int kSEM_FUNCTION = 4;
static constexpr int kSEM_CLASS    = 5;
static constexpr int kSEM_COMMENT  = 6;
static constexpr int kSEM_OPERATOR = 7;

static inline int token_to_semantic(TokenType type) noexcept {
  switch (type) {
  // Keywords
  case TokenType::TOKEN_ABSTRACT:
  case TokenType::TOKEN_AND:
  case TokenType::TOKEN_BREAK:
  case TokenType::TOKEN_CLASS:
  case TokenType::TOKEN_CONTINUE:
  case TokenType::TOKEN_ELSE:
  case TokenType::TOKEN_FALSE:
  case TokenType::TOKEN_FOR:
  case TokenType::TOKEN_FUN:
  case TokenType::TOKEN_IF:
  case TokenType::TOKEN_IN:
  case TokenType::TOKEN_IMPORT:
  case TokenType::TOKEN_FROM:
  case TokenType::TOKEN_AS:
  case TokenType::TOKEN_NIL:
  case TokenType::TOKEN_OR:
  case TokenType::TOKEN_PRINT:
  case TokenType::TOKEN_RETURN:
  case TokenType::TOKEN_STATIC:
  case TokenType::TOKEN_SUPER:
  case TokenType::TOKEN_THIS:
  case TokenType::TOKEN_THROW:
  case TokenType::TOKEN_TRUE:
  case TokenType::TOKEN_TRY:
  case TokenType::TOKEN_VAR:
  case TokenType::TOKEN_SWITCH:
  case TokenType::TOKEN_WHILE:
  case TokenType::TOKEN_CASE:
  case TokenType::TOKEN_CATCH:
  case TokenType::TOKEN_DEFAULT:
  case TokenType::TOKEN_DEFER:
    return kSEM_KEYWORD;

  // Strings
  case TokenType::TOKEN_STRING:
  case TokenType::TOKEN_STRING_INTERP:
    return kSEM_STRING;

  // Numbers
  case TokenType::TOKEN_NUMBER:
  case TokenType::TOKEN_INTEGER:
    return kSEM_NUMBER;

  // Identifiers (may be upgraded to function/class by context)
  case TokenType::TOKEN_IDENTIFIER:
    return kSEM_VARIABLE;

  // Operators
  case TokenType::TOKEN_PLUS:
  case TokenType::TOKEN_MINUS:
  case TokenType::TOKEN_STAR:
  case TokenType::TOKEN_SLASH:
  case TokenType::TOKEN_PERCENT:
  case TokenType::TOKEN_BANG:
  case TokenType::TOKEN_BANG_EQUAL:
  case TokenType::TOKEN_EQUAL:
  case TokenType::TOKEN_EQUAL_EQUAL:
  case TokenType::TOKEN_GREATER:
  case TokenType::TOKEN_GREATER_EQUAL:
  case TokenType::TOKEN_LESS:
  case TokenType::TOKEN_LESS_EQUAL:
  case TokenType::TOKEN_AMPERSAND:
  case TokenType::TOKEN_PIPE:
  case TokenType::TOKEN_CARET:
  case TokenType::TOKEN_TILDE:
  case TokenType::TOKEN_LEFT_SHIFT:
  case TokenType::TOKEN_RIGHT_SHIFT:
    return kSEM_OPERATOR;

  default:
    return -1; // Not a semantic token
  }
}

void LspServer::handle_semantic_tokens_full(
    const json::Value& id, const json::Value& params) noexcept {
  auto uri = params["textDocument"]["uri"].as_string();
  auto it = documents_.find(uri);
  if (it == documents_.end()) {
    send_response(id, json::Value::object());
    return;
  }
  send_response(id, compute_semantic_tokens(it->second));
}

json::Value LspServer::compute_semantic_tokens(const str_t& source) noexcept {
  Scanner scanner(source);
  auto data = json::Value::array();

  int prev_line = 0;
  int prev_col = 0;
  TokenType prev_type = TokenType::TOKEN_EOF;

  for (;;) {
    Token tok = scanner.scan_token();
    if (tok.type == TokenType::TOKEN_EOF) break;
    if (tok.type == TokenType::TOKEN_ERROR) continue;

    int sem_type = token_to_semantic(tok.type);

    // Context-sensitive: identifier after `fun` is a function declaration
    if (tok.type == TokenType::TOKEN_IDENTIFIER) {
      if (prev_type == TokenType::TOKEN_FUN)
        sem_type = kSEM_FUNCTION;
      else if (prev_type == TokenType::TOKEN_CLASS)
        sem_type = kSEM_CLASS;
    }

    if (sem_type < 0) {
      prev_type = tok.type;
      continue;
    }

    int line = tok.line - 1;   // 0-based
    int col = tok.column - 1;  // 0-based
    int length = static_cast<int>(tok.lexeme.size());

    int delta_line = line - prev_line;
    int delta_col = (delta_line == 0) ? (col - prev_col) : col;

    data.push(json::Value(delta_line));
    data.push(json::Value(delta_col));
    data.push(json::Value(length));
    data.push(json::Value(sem_type));
    data.push(json::Value(0)); // modifiers bitmask

    prev_line = line;
    prev_col = col;
    prev_type = tok.type;
  }

  auto result = json::Value::object();
  result["data"] = std::move(data);
  return result;
}

// =============================================================================
// Document symbols
// =============================================================================

// LSP SymbolKind values
static constexpr int kSYM_FILE     = 1;
static constexpr int kSYM_CLASS    = 5;
static constexpr int kSYM_FUNCTION = 12;
static constexpr int kSYM_VARIABLE = 13;

void LspServer::handle_document_symbol(
    const json::Value& id, const json::Value& params) noexcept {
  auto uri = params["textDocument"]["uri"].as_string();
  auto it = documents_.find(uri);
  if (it == documents_.end()) {
    send_response(id, json::Value::array());
    return;
  }
  send_response(id, compute_document_symbols(it->second));
}

static inline json::Value make_position(int line, int character) noexcept {
  auto pos = json::Value::object();
  pos["line"] = json::Value(line);
  pos["character"] = json::Value(character);
  return pos;
}

static inline json::Value make_range(int line, int col, int end_col) noexcept {
  auto range = json::Value::object();
  range["start"] = make_position(line, col);
  range["end"] = make_position(line, end_col);
  return range;
}

json::Value LspServer::compute_document_symbols(const str_t& source) noexcept {
  Scanner scanner(source);
  auto symbols = json::Value::array();

  int brace_depth = 0;
  TokenType prev_type = TokenType::TOKEN_EOF;

  for (;;) {
    Token tok = scanner.scan_token();
    if (tok.type == TokenType::TOKEN_EOF) break;
    if (tok.type == TokenType::TOKEN_ERROR) continue;

    if (tok.type == TokenType::TOKEN_LEFT_BRACE) ++brace_depth;
    if (tok.type == TokenType::TOKEN_RIGHT_BRACE) --brace_depth;

    // Only capture top-level declarations
    if (brace_depth == 0 && tok.type == TokenType::TOKEN_IDENTIFIER) {
      int kind = -1;
      if (prev_type == TokenType::TOKEN_FUN) kind = kSYM_FUNCTION;
      else if (prev_type == TokenType::TOKEN_CLASS) kind = kSYM_CLASS;
      else if (prev_type == TokenType::TOKEN_VAR) kind = kSYM_VARIABLE;

      if (kind >= 0) {
        int line = tok.line - 1;
        int col = tok.column - 1;
        int end_col = col + static_cast<int>(tok.lexeme.size());

        auto sym = json::Value::object();
        sym["name"] = json::Value(str_t(tok.lexeme));
        sym["kind"] = json::Value(kind);
        sym["range"] = make_range(line, col, end_col);
        sym["selectionRange"] = make_range(line, col, end_col);
        symbols.push(std::move(sym));
      }
    }

    prev_type = tok.type;
  }

  return symbols;
}

} // namespace ms
