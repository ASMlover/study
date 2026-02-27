#include "minicli4/tui.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#if defined(MINICLI4_HAVE_FTXUI)
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#endif

namespace minicli4 {

namespace {
std::string ansi_wrap(const std::string& code, const std::string& text) {
  return "\x1b[" + code + "m" + text + "\x1b[0m";
}

std::string gray(const std::string& s) { return ansi_wrap("90", s); }
std::string blue(const std::string& s) { return ansi_wrap("34", s); }
std::string green(const std::string& s) { return ansi_wrap("32", s); }
std::string yellow(const std::string& s) { return ansi_wrap("33", s); }
std::string magenta(const std::string& s) { return ansi_wrap("35", s); }
std::string cyan(const std::string& s) { return ansi_wrap("36", s); }
std::string bold(const std::string& s) { return ansi_wrap("1", s); }

std::string now_hms() {
  const std::time_t t = std::time(nullptr);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  std::ostringstream out;
  out << std::put_time(&tm, "%H:%M:%S");
  return out.str();
}

std::string pad2(int value) {
  std::ostringstream out;
  out << std::setw(2) << std::setfill('0') << value;
  return out.str();
}

std::vector<std::string> split_words(const std::string& text) {
  std::istringstream in(text);
  std::vector<std::string> words;
  std::string word;
  while (in >> word) {
    words.push_back(word);
  }
  return words;
}

std::string animate_detail_words(const std::string& detail, int frame) {
  const auto words = split_words(detail);
  if (words.size() <= 1) {
    return gray(detail);
  }
  const size_t hi = static_cast<size_t>(frame) % words.size();
  const bool breathe = ((frame / 2) % 2) == 0;
  std::ostringstream out;
  for (size_t i = 0; i < words.size(); i++) {
    if (i > 0) {
      out << " ";
    }
    if (i == hi) {
      const std::string emphasis = breathe ? yellow(words[i]) : cyan(words[i]);
      out << bold(emphasis);
    } else {
      out << gray(words[i]);
    }
  }
  return out.str();
}

std::string pulse_text(const std::string& text, int frame, int period) {
  if (period <= 0) {
    return text;
  }
  const bool on = (frame % period) == 0;
  return on ? bold(text) : text;
}

#if defined(MINICLI4_HAVE_FTXUI)
std::string render_ftxui_lines(const std::vector<std::string>& lines) {
  using namespace ftxui;
  Elements text_lines;
  text_lines.reserve(lines.size());
  for (const auto& line : lines) {
    text_lines.push_back(text(line));
  }
  auto doc = vbox(std::move(text_lines));
  auto screen = Screen::Create(Dimension::Fit(doc));
  Render(screen, doc);
  return screen.ToString();
}
#endif

void write_line(std::ostream& output, bool use_ftxui, const std::string& line) {
#if defined(MINICLI4_HAVE_FTXUI)
  if (use_ftxui) {
    output << render_ftxui_lines({line});
    return;
  }
#endif
  output << line << "\n";
}

void write_lines(std::ostream& output, bool use_ftxui, const std::vector<std::string>& lines) {
#if defined(MINICLI4_HAVE_FTXUI)
  if (use_ftxui) {
    output << render_ftxui_lines(lines);
    return;
  }
#endif
  for (const auto& line : lines) {
    output << line << "\n";
  }
}

}  // namespace

bool TwoPaneTui::ftxui_available() {
#if defined(MINICLI4_HAVE_FTXUI)
  return true;
#else
  return false;
#endif
}

TwoPaneTui::TwoPaneTui(std::ostream& output, bool is_tty) : output_(output), is_tty_(is_tty) {
  if (const char* stage_frames = std::getenv("MINICLI4_TUI_STAGE_FRAMES")) {
    try {
      const int parsed = std::stoi(stage_frames);
      if (parsed > 0) {
        stage_frames_ = parsed;
      }
    } catch (...) {
      stage_frames_ = 6;
    }
  }
#if defined(MINICLI4_HAVE_FTXUI)
  const char* env_backend = std::getenv("MINICLI4_TUI_BACKEND");
  const std::string requested = env_backend ? std::string(env_backend) : "";
  if (!is_tty_) {
    use_ftxui_ = false;
  } else if (requested == "ansi") {
    use_ftxui_ = false;
  } else {
    use_ftxui_ = true;
  }
#else
  use_ftxui_ = false;
#endif
}

TwoPaneTui::~TwoPaneTui() {
  stop_thinking();
}

std::string TwoPaneTui::backend_name() const {
  return use_ftxui_ ? "ftxui" : "ansi";
}

void TwoPaneTui::start(const PaneState& status) {
  if (!is_tty_) {
    return;
  }
  const std::string divider(72, '-');
  write_lines(
      output_,
      use_ftxui_,
      {
          bold(cyan("MiniCLI4")) + " " + gray("C++ Agent CLI"),
          gray("Mode:") + " " + green(status.mode) + "  " + gray("Model:") + " " + magenta(status.model) + "  " + gray("Theme:") +
              " " + yellow(theme_) + "  " + gray("Motion:") + " " + yellow(motion_) + "  " + gray("Session:") + " " + cyan(status.session_id),
          gray("Keys:") + " " + blue("Tab") + " complete | " + blue("Ctrl+C") + " stop | " + blue("Ctrl+L") + " status | " + blue("F1") +
              " help | " + blue("Esc") + " close menu",
          gray(divider),
      });
  last_signature_ = signature(status);
}

void TwoPaneTui::announce_input(const std::string& input_text) {
  if (!is_tty_) {
    return;
  }
  stop_thinking();
  end_assistant_stream();
  turn_ += 1;
  write_line(output_, use_ftxui_, "");
  const std::string who = "[YOU " + pad2(turn_) + "]";
  write_line(output_, use_ftxui_, gray("[" + now_hms() + "]") + " " + green(who) + " " + bold(input_text));
}

void TwoPaneTui::print_event(const std::string& line) {
  if (!is_tty_) {
    return;
  }
  stop_thinking();
  end_assistant_stream();
  write_line(output_, use_ftxui_, line);
}

void TwoPaneTui::update_status(const PaneState& status) {
  if (!is_tty_) {
    return;
  }
  last_signature_ = signature(status);
}

void TwoPaneTui::print_status(const PaneState& status) {
  if (!is_tty_) {
    return;
  }
  stop_thinking();
  end_assistant_stream();
  write_line(
      output_,
      use_ftxui_,
      yellow("[STATUS]") + " " + gray("session=") + cyan(status.session_id) + " " + gray("model=") + magenta(status.model) + " " +
          gray("stage=") + blue(status.stage) + " " + gray("mode=") + green(status.mode) + " " + gray("approval=") + yellow(status.pending_approval));
  last_signature_ = signature(status);
}

void TwoPaneTui::start_assistant_stream() {
  if (!is_tty_) {
    return;
  }
  stop_thinking();
  if (stream_open_) {
    return;
  }
  stream_open_ = true;
}

void TwoPaneTui::append_assistant_chunk(const std::string& chunk) {
  if (!is_tty_) {
    return;
  }
  if (!stream_open_) {
    start_assistant_stream();
  }
  output_ << chunk << std::flush;
}

void TwoPaneTui::end_assistant_stream() {
  if (!is_tty_ || !stream_open_) {
    return;
  }
  output_ << "\n";
  output_ << std::flush;
  stream_open_ = false;
}

void TwoPaneTui::start_thinking() {
  if (!is_tty_ || thinking_active_) {
    return;
  }
  end_assistant_stream();
  thinking_active_ = true;
  thinking_frame_ = 0;
  thinking_stop_.store(false);
  output_ << "\r\x1b[2K" << render_thinking_frame();
  output_ << std::flush;
  const auto interval = motion_ == "minimal" ? std::chrono::milliseconds(480) : std::chrono::milliseconds(160);
  thinking_thread_ = std::thread([this, interval]() {
    while (!thinking_stop_.load()) {
      std::this_thread::sleep_for(interval);
      if (thinking_stop_.load() || !thinking_active_) {
        return;
      }
      thinking_frame_ += 1;
      output_ << "\r\x1b[2K" << render_thinking_frame();
      output_ << std::flush;
    }
  });
}

void TwoPaneTui::tick_thinking() {
  if (!is_tty_ || !thinking_active_) {
    return;
  }
  thinking_frame_ += 1;
  output_ << "\r\x1b[2K" << render_thinking_frame();
  output_ << std::flush;
}

void TwoPaneTui::stop_thinking() {
  if (!is_tty_ || !thinking_active_) {
    return;
  }
  thinking_stop_.store(true);
  if (thinking_thread_.joinable()) {
    thinking_thread_.join();
  }
  thinking_active_ = false;
  output_ << "\r\x1b[2K";
  output_ << std::flush;
}

void TwoPaneTui::apply_config(const RuntimeConfig& config) {
  motion_ = config.motion;
  theme_ = config.theme;
}

std::string TwoPaneTui::signature(const PaneState& status) const {
  return status.session_id + "|" + status.model + "|" + status.stage + "|" + status.mode + "|" + status.pending_approval;
}

std::string TwoPaneTui::render_thinking_frame() const {
  struct Stage {
    std::string name;
    std::string icon;
    std::vector<std::string> details;
  };
  static const std::vector<std::string> logo_frames = {"<> MiniCLI4 <>", "[] MiniCLI4 []", "{} MiniCLI4 {}", "() MiniCLI4 ()"};
  static const std::vector<std::string> aura_frames = {"*", ".", "o", "."};
  static const std::vector<Stage> stages = {
      {"Thinking", "TH", {"collecting context", "reviewing intent", "loading constraints"}},
      {"Reasoning", "RE", {"evaluating options", "checking tradeoffs", "validating assumptions"}},
      {"Drafting", "DR", {"structuring response", "building answer", "assembling details"}},
      {"Refining", "RF", {"improving clarity", "tightening phrasing", "verifying coherence"}},
      {"Finalizing", "FI", {"preparing stream", "readying output", "sending response"}},
  };

  const auto idx = static_cast<size_t>((thinking_frame_ / stage_frames_) % static_cast<int>(stages.size()));
  const auto& stage = stages[idx];
  const auto& detail = stage.details[static_cast<size_t>(thinking_frame_) % stage.details.size()];

  const int elapsed_ms = thinking_frame_ * (motion_ == "minimal" ? 480 : 160);
  const int elapsed_sec = elapsed_ms / 1000;
  const int mm = elapsed_sec / 60;
  const int ss = elapsed_sec % 60;

  const std::string logo = cyan(logo_frames[static_cast<size_t>(thinking_frame_) % logo_frames.size()]);
  const std::string aura = yellow(aura_frames[static_cast<size_t>(thinking_frame_) % aura_frames.size()]);
  const std::string padded_stage = stage.name + std::string(stage.name.size() < 10 ? 10 - stage.name.size() : 0, ' ');
  const std::string stage_icon_base = (thinking_frame_ % 3 == 0) ? magenta(stage.icon) : blue(stage.icon);
  const std::string stage_icon = pulse_text(stage_icon_base, thinking_frame_, 4);
  const std::string stage_colorized = (thinking_frame_ % 2 == 0) ? blue(padded_stage) : magenta(padded_stage);
  const std::string stage_text = pulse_text(stage_colorized, thinking_frame_ + 1, 5);
  const std::string timer = gray(pad2(mm) + ":" + pad2(ss));
  const std::string detail_text = motion_ == "minimal" ? gray(detail) : animate_detail_words(detail, thinking_frame_);

  if (motion_ == "minimal") {
    return logo + " " + stage_icon + " " + stage_text + " " + timer + " " + detail_text;
  }
  return logo + " " + aura + " " + stage_icon + " " + stage_text + " " + timer + " " + aura + " " + detail_text;
}

}  // namespace minicli4
