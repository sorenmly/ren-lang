#include "log.hpp"
#include <iostream>

Logger::LOG_LEVEL Logger::min_level = Logger::INFO;

void Logger::log_section(LOG_MODULE module) {
  std::string mo;

  switch (module) {
  case LEXER:
    mo = "LEXER";
    break;
  case PARSER:
    mo = "PARSER";
    break;
  case INTERPRETER:
    mo = "INTERPRETER";
    break;
  }

  std::cout << "+---- " << mo << " ----+" << std::endl;
}

void Logger::log(LOG_LEVEL level, LOG_MODULE module, std::string message) {
  std::string color;
  std::string level_str;

  if (level < min_level && level != OFF)
    return;

  switch (level) {
  case INFO:
    color = "\033[32m";
    level_str = "[INFO]";
    break;
  case WARN:
    color = "\033[33m";
    level_str = "[WARN]";
    break;
  case ERROR:
    color = "\033[31m";
    level_str = "[ERROR]";
    break;
  case OFF:
    break;
  }

  switch (module) {
  case LEXER:
    std::cout << color << level_str
              << " | \033[34m>. Lexer: \033[m" + color + message + "\033[0m" +
                     '\n';
    break;
  case PARSER:
    std::cout << color << level_str
              << " | \033[35m>. Parser: \033[m" + color + message + "\033[0m" +
                     '\n';
    break;
  case INTERPRETER:
    std::cout << color << level_str
              << " | \033[36m >. Interpreter: \033[m" + color + message +
                     "\033[0m" + '\n';
    break;
  }
}
