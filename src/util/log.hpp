#ifndef LOG_HPP
#define LOG_HPP

#include <string>

class Logger {
public:
  enum LOG_LEVEL { INFO, WARN, ERROR, OFF };
  enum LOG_MODULE { LEXER, PARSER, INTERPRETER };

  static LOG_LEVEL min_level;

  //@brief Logs an information based on a level and module
  //@param level The level from the enum LOG_LEVEL
  //@param module The moduel from the enum LOG_MODULE
  //@param message The message in std::string
  static void log(LOG_LEVEL level, LOG_MODULE module, std::string message);

  //@brief Prints a section divisor based on module
  //@param module The module to be print in the divisor
  static void log_section(LOG_MODULE module);

  Logger() = delete;
};

#endif
