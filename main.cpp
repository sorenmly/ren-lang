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

  for (auto &[key, val] : interp.get_node_map()) {
    std::string val_str;
    std::visit(
        [&val_str](auto &&v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, std::string>)
            val_str = v;
          else
            val_str = std::to_string(v);
        },
        val);
    Logger::log(Logger::INFO, Logger::INTERPRETER,
                "\033[40mvarDecl\033[0m " + key + " = " + val_str);
  }
  return 0;
}
