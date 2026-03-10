#include "test_common.hh"

#include <sstream>

#include "support/logger.hh"

int RunLoggerTests() {
  ms::Logger& logger = ms::Logger::instance();
  std::ostringstream out;
  logger.set_sink(out);
  logger.set_color_enabled(false);
  logger.set_min_level(ms::LogLevel::kInfo);
  logger.log(ms::LogLevel::kDebug, "hidden");
  logger.log(ms::LogLevel::kInfo, "visible");
  Expect(out.str().find("hidden") == std::string::npos, "debug should be filtered");
  Expect(out.str().find("[INFO] visible") != std::string::npos,
         "info should be emitted");
  return 0;
}
