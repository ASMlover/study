#include "test_common.hh"

#include <sstream>

#include "support/logger.hh"

int RunLoggerTests() {
  ms::Logger& logger = ms::Logger::Instance();
  std::ostringstream out;
  logger.SetSink(out);
  logger.SetColorEnabled(false);
  logger.SetMinLevel(ms::LogLevel::kInfo);
  logger.Log(ms::LogLevel::kDebug, "hidden");
  logger.Log(ms::LogLevel::kInfo, "visible");
  Expect(out.str().find("hidden") == std::string::npos, "debug should be filtered");
  Expect(out.str().find("[INFO] visible") != std::string::npos,
         "info should be emitted");
  return 0;
}

