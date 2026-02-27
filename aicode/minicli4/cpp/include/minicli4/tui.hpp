#pragma once

#include <atomic>
#include <iosfwd>
#include <string>
#include <thread>

#include "minicli4/config.hpp"

namespace minicli4 {

struct PaneState {
  std::string session_id;
  std::string model;
  std::string stage;
  std::string tools;
  std::string mode;
  std::string pending_approval;
};

class TwoPaneTui {
 public:
  TwoPaneTui(std::ostream& output, bool is_tty);
  ~TwoPaneTui();
  static bool ftxui_available();
  std::string backend_name() const;

  void start(const PaneState& status);
  void announce_input(const std::string& input_text);
  void print_event(const std::string& line);
  void update_status(const PaneState& status);
  void print_status(const PaneState& status);
  void start_assistant_stream();
  void append_assistant_chunk(const std::string& chunk);
  void end_assistant_stream();
  void start_thinking();
  void tick_thinking();
  void stop_thinking();
  void apply_config(const RuntimeConfig& config);

 private:
  std::ostream& output_;
  bool is_tty_;
  bool use_ftxui_{false};
  bool stream_open_{false};
  bool thinking_active_{false};
  int thinking_frame_{0};
  std::string motion_{"full"};
  std::string theme_{"dark"};
  int stage_frames_{6};
  int turn_{0};
  std::string last_signature_;
  std::thread thinking_thread_;
  std::atomic<bool> thinking_stop_{false};

  std::string signature(const PaneState& status) const;
  std::string render_thinking_frame() const;
};

}  // namespace minicli4
