#include "minicli4/cli.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "minicli4/agent.hpp"
#include "minicli4/commands.hpp"
#include "minicli4/completion.hpp"
#include "minicli4/config.hpp"
#include "minicli4/provider.hpp"
#include "minicli4/session.hpp"
#include "minicli4/tools.hpp"
#include "minicli4/tui.hpp"

#if defined(_WIN32)
#include <conio.h>
#include <io.h>
#include <windows.h>
#define MINICLI4_ISATTY _isatty
#define MINICLI4_FILENO _fileno
#else
#include <termios.h>
#include <unistd.h>
#define MINICLI4_ISATTY isatty
#define MINICLI4_FILENO fileno
#endif

namespace minicli4 {

namespace {
std::string trim_copy(const std::string& s) {
  size_t begin = 0;
  while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])) != 0) {
    begin++;
  }
  size_t end = s.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
    end--;
  }
  return s.substr(begin, end - begin);
}

bool stdout_is_tty() {
  return MINICLI4_ISATTY(MINICLI4_FILENO(stdout)) != 0;
}

bool stdin_is_tty() {
  return MINICLI4_ISATTY(MINICLI4_FILENO(stdin)) != 0;
}

#if defined(_WIN32)
void configure_windows_console() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE || hOut == nullptr) {
    return;
  }
  DWORD mode = 0;
  if (!GetConsoleMode(hOut, &mode)) {
    return;
  }
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, mode);
}
#endif

std::string prompt_text(bool tty) {
  if (!tty) {
    return "> ";
  }
  return "\x1b[36m>\x1b[0m ";
}

std::vector<std::string> split_ws(const std::string& s) {
  std::istringstream in(s);
  std::vector<std::string> out;
  std::string item;
  while (in >> item) {
    out.push_back(item);
  }
  return out;
}

void redraw_prompt(const std::string& prompt, const std::string& buffer) {
  std::cout << "\r\x1b[2K" << prompt << buffer << std::flush;
}

std::string apply_single_completion(const std::string& buffer, const std::string& candidate) {
  const bool trailing = !buffer.empty() && buffer.back() == ' ';
  if (trailing) {
    return buffer + candidate;
  }
  const auto last_space = buffer.find_last_of(' ');
  if (last_space == std::string::npos) {
    return candidate;
  }
  return buffer.substr(0, last_space + 1) + candidate;
}

std::pair<bool, std::string> read_line_with_completion(
    const std::string& prompt,
    bool tty,
    const std::function<std::vector<std::string>(const std::string&)>& completer) {
  if (!tty) {
    std::string line;
    if (!std::getline(std::cin, line)) {
      return {false, {}};
    }
    return {true, line};
  }

#if defined(_WIN32)
  auto wide_to_utf8 = [](const std::wstring& w) -> std::string {
    if (w.empty()) {
      return {};
    }
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) {
      return {};
    }
    std::string out(static_cast<size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), out.data(), bytes, nullptr, nullptr);
    return out;
  };
  auto utf8_to_wide = [](const std::string& s) -> std::wstring {
    if (s.empty()) {
      return {};
    }
    const int wchars = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    if (wchars <= 0) {
      return {};
    }
    std::wstring out(static_cast<size_t>(wchars), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), out.data(), wchars);
    return out;
  };

  const char* line_mode = std::getenv("MINICLI4_WIN_LINE_INPUT");
  const bool use_line_input = line_mode != nullptr && std::string(line_mode) == "1";
  if (use_line_input) {
    std::cout << prompt << std::flush;
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hIn == nullptr) {
      return {false, {}};
    }
    std::wstring wline(4096, L'\0');
    DWORD read = 0;
    if (!ReadConsoleW(hIn, wline.data(), static_cast<DWORD>(wline.size() - 1), &read, nullptr)) {
      return {false, {}};
    }
    if (read == 0) {
      return {false, {}};
    }
    wline.resize(static_cast<size_t>(read));
    while (!wline.empty() && (wline.back() == L'\r' || wline.back() == L'\n')) {
      wline.pop_back();
    }
    return {true, wide_to_utf8(wline)};
  }

  std::wstring wbuffer;
  std::cout << prompt << std::flush;
  while (true) {
    const wint_t ch = _getwch();
    if (ch == 3) {
      std::cout << "^C\n";
      return {true, "/exit"};
    }
    if (ch == L'\r') {
      std::cout << "\n";
      return {true, wide_to_utf8(wbuffer)};
    }
    if (ch == L'\b' || ch == 127) {
      if (!wbuffer.empty()) {
        wbuffer.pop_back();
        redraw_prompt(prompt, wide_to_utf8(wbuffer));
      }
      continue;
    }
    if (ch == L'\t') {
      const std::string current = wide_to_utf8(wbuffer);
      const auto items = completer(current);
      if (items.empty()) {
        continue;
      }
      if (items.size() == 1) {
        wbuffer = utf8_to_wide(apply_single_completion(current, items.front()));
        redraw_prompt(prompt, wide_to_utf8(wbuffer));
      } else {
        std::cout << "\n";
        for (const auto& item : items) {
          std::cout << item << "  ";
        }
        std::cout << "\n";
        redraw_prompt(prompt, wide_to_utf8(wbuffer));
      }
      continue;
    }
    if (ch == 0 || ch == 0xE0) {
      (void)_getwch();
      continue;
    }
    if (ch >= 32) {
      wbuffer.push_back(static_cast<wchar_t>(ch));
      redraw_prompt(prompt, wide_to_utf8(wbuffer));
    }
  }
#else
  std::string line;
  std::cout << prompt << std::flush;
  if (!std::getline(std::cin, line)) {
    return {false, {}};
  }
  if (line.find('\t') != std::string::npos) {
    std::string probe = line;
    probe.erase(std::remove(probe.begin(), probe.end(), '\t'), probe.end());
    const auto items = completer(probe);
    if (items.size() == 1) {
      line = apply_single_completion(probe, items.front());
    }
  }
  return {true, line};
#endif
}

}  // namespace

int run_cli(const std::vector<std::string>& argv) {
#if defined(_WIN32)
  configure_windows_console();
#endif
  if (argv.size() == 1 && (argv[0] == "-v" || argv[0] == "--version")) {
    std::cout << "minicli4-cpp 0.1.0\n";
    return 0;
  }

  const bool tty_out = stdout_is_tty();
  const bool tty_in = stdin_is_tty();
  const bool interactive = tty_out && tty_in;

  if (argv.size() == 1 && (argv[0] == "backend" || argv[0] == "backend-check")) {
    TwoPaneTui probe(std::cout, interactive);
    std::cout << "tui_backend=" << probe.backend_name()
              << " ftxui_available=" << (TwoPaneTui::ftxui_available() ? "true" : "false") << "\n";
    return 0;
  }
  if (!argv.empty() && argv[0] != "repl") {
    std::cerr << "Unknown argument: " << argv[0] << "\n";
    return 1;
  }

  const auto project_root = std::filesystem::current_path();
  auto loaded = load_config(project_root);
  RuntimeConfig config = loaded.config;
  SessionStore sessions;
  SessionRecord session = sessions.current();
  ToolRegistry tools(project_root, config);
  GLMProvider provider(config);
  TwoPaneTui tui(std::cout, interactive);
  std::string pending;

  PaneState pane{
      session.session_id,
      config.model,
      "idle",
      "",
      config.safe_mode,
      "none",
  };
  pane.tools = [&]() {
    auto names = tools.list_tool_names();
    std::ostringstream out;
    for (size_t i = 0; i < names.size(); i++) {
      if (i > 0) {
        out << ",";
      }
      out << names[i];
    }
    return out.str();
  }();

  auto emit_line = [&](const std::string& line) {
    if (interactive) {
      tui.print_event(line);
      return;
    }
    std::cout << line << "\n";
  };

  auto set_config = [&](const RuntimeConfig& next) {
    config = next;
    tools = ToolRegistry(project_root, config);
    provider = GLMProvider(config);
    pane.model = config.model;
    pane.mode = config.safe_mode;
    auto names = tools.list_tool_names();
    std::ostringstream out;
    for (size_t i = 0; i < names.size(); i++) {
      if (i > 0) {
        out << ",";
      }
      out << names[i];
    }
    pane.tools = out.str();
    tui.apply_config(config);
    tui.update_status(pane);
  };

  auto set_session = [&](const SessionRecord& next) {
    session = next;
    pane.session_id = next.session_id;
    tui.update_status(pane);
  };

  CommandContext ctx{
      project_root,
      &config,
      &session,
      &sessions,
      &tools,
      set_config,
      set_session,
      [&](const std::string& line) { emit_line(line); },
      [&](const std::string& value) {
        pending = value;
        pane.pending_approval = value;
        tui.update_status(pane);
      },
      [&]() {
        pending.clear();
        pane.pending_approval = "none";
        tui.update_status(pane);
      },
      [&]() { return pending; },
  };

  if (interactive) {
    tui.apply_config(config);
    tui.start(pane);
  }
  std::cout << "[SELF-CHECK] tui_backend=" << tui.backend_name()
            << " ftxui_available=" << (TwoPaneTui::ftxui_available() ? "true" : "false") << "\n";
  std::cout << "MiniCLI4 C++ REPL. Type /help or /exit.\n";

  const auto completer = [&](const std::string& line) {
    return complete(
        line,
        CompletionContext{
            commands(),
            [&]() {
              std::vector<std::string> ids;
              for (const auto& rec : sessions.list()) {
                ids.push_back(rec.session_id);
              }
              if (ids.empty()) {
                ids.push_back("default");
              }
              return ids;
            }(),
            {"api_key", "base_url", "model", "stream", "timeout_ms", "max_retries", "temperature", "max_tokens", "agent_max_rounds", "safe_mode", "theme", "motion", "allowed_paths", "shell_allowlist", "slash_completion_mode"},
            tools.list_tool_names(),
            project_root,
            {},
        });
  };

  while (true) {
    const auto [ok, raw] = read_line_with_completion(prompt_text(interactive), interactive, completer);
    if (!ok) {
      break;
    }
    const std::string input = trim_copy(raw);
    if (input.empty()) {
      continue;
    }

    if (interactive) {
      tui.announce_input(input);
    }

    if (!input.empty() && input[0] == '/') {
      const bool keep = run_slash(ctx, input);
      if (!keep) {
        break;
      }
      continue;
    }

    session.messages.push_back(SessionMessage{"user", input, now_iso(), "", ""});
    sessions.save(session);

    pane.stage = "planner";
    tui.update_status(pane);
    if (interactive) {
      tui.start_thinking();
    }

    bool stream_started = false;
    std::string final;
    try {
      const auto result = run_multi_agent_round(
          input,
          session.messages,
          provider,
          tools,
          config,
          AgentCallbacks{
              [&](const std::string& stage, const std::string&) {
                pane.stage = stage;
                tui.update_status(pane);
              },
              [&](const std::string&, const std::string&) {},
              [&](const std::string& chunk) {
                if (!stream_started) {
                  stream_started = true;
                  if (interactive) {
                    tui.stop_thinking();
                    tui.start_assistant_stream();
                  }
                }
                if (interactive) {
                  tui.append_assistant_chunk(chunk);
                } else {
                  std::cout << chunk;
                }
              },
          });
      final = result.final;
    } catch (const std::exception& ex) {
      if (interactive) {
        tui.stop_thinking();
      }
      std::cerr << "[error] " << ex.what() << "\n";
      pane.stage = "error";
      tui.update_status(pane);
      continue;
    }

    if (stream_started) {
      if (interactive) {
        tui.stop_thinking();
        tui.end_assistant_stream();
        const char* echo_final = std::getenv("MINICLI4_STREAM_ECHO_FINAL");
        const bool force_echo_final = echo_final == nullptr || std::string(echo_final) != "0";
        if (force_echo_final && !final.empty()) {
          tui.print_event(final);
        }
      } else {
        std::cout << "\n";
      }
    } else {
      if (interactive) {
        tui.stop_thinking();
        tui.start_assistant_stream();
        tui.append_assistant_chunk(final);
        tui.end_assistant_stream();
      } else {
        std::cout << final << "\n";
      }
    }

    session.messages.push_back(SessionMessage{"assistant", final, now_iso(), "", ""});
    sessions.save(session);
    pane.stage = "idle";
    tui.update_status(pane);
  }

  std::cout << "Bye.\n";
  return 0;
}

}  // namespace minicli4
