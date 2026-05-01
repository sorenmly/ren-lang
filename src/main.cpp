#include "interpreter/interpreter.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "util/log.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
  std::ifstream file("main.mly");
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string src = buffer.str();
  Lexer l = Lexer(src);

  Logger::log_section(Logger::LEXER);
  std::vector<Token> a = l.lexer_process();
  l.debug_tokens(a);

  Logger::log_section(Logger::PARSER);

  Parser p(a);

  // try {
  std::vector<Node> nodes = p.parse();
  p.debug_tree(nodes);

  // } catch (std::exception &e) {
  //   std::cout << "err: " << e.what() << std::endl;
  // }

  Logger::log_section(Logger::INTERPRETER);

  Interpreter interp;
  interp.execute(nodes);

  Logger::log(Logger::INFO, Logger::INTERPRETER, "starting var dump...");
  for (auto &[key, val] : interp.get_node_map()) {
    Logger::log(Logger::INFO, Logger::INTERPRETER, "dumping: " + key);
    std::string val_str;
    std::visit(
        [&val_str](auto &&v) {
          using VT = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<VT, NumberExprNode>)
            val_str = std::to_string(v.value);
          else if constexpr (std::is_same_v<VT, StringExprNode>)
            val_str = v.value;
          else
            val_str = "?";
        },
        val.value);
    Logger::log(Logger::INFO, Logger::INTERPRETER,
                "varDecl " + key + " = " + val_str);
  }
  Logger::log(Logger::INFO, Logger::INTERPRETER, "dump complete!");
  return 0;
}
