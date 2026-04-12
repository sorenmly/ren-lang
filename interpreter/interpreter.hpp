#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP
#include "../parser/parser.hpp"
#include <unordered_map>

class Interpreter {
private:
  std::unordered_map<std::string, Value> node_map;

public:
  /* starts blank */
  Interpreter() : node_map(std::unordered_map<std::string, Value>{}) {}

  //@brief Returns the node_map, these being an unordered_map.
  //@returns unordered_map<std::string, Value>, where Value is from the parser.
  std::unordered_map<std::string, Value> get_node_map();

  //@brief Execute running through the given AST
  //@param nodes A vector of nodes from structure Node obtained with the Parser.
  void execute(std::vector<Node> nodes);
};
#endif
