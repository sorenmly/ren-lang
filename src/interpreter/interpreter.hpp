#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP
#include "../parser/parser.hpp"
#include <unordered_map>

class Interpreter {
private:
  std::unordered_map<std::string, Expression> node_map;
  std::unordered_map<std::string, FnDeclNode> fn_map;
  bool has_return = false;
  Value return_value{0.0f};

public:
  /* starts blank */
  Interpreter() : node_map(std::unordered_map<std::string, Expression>()) {}

  //@brief Returns the node_map, these being an unordered_map.
  //@returns unordered_map<std::string, Expression>, where Value is from the
  // parser.
  std::unordered_map<std::string, Expression> get_node_map();

  //@brief Evaluates an expression.
  //@param expr The expresison from the struct Expression.
  //@returns It returns a float, int or string depending the token. Value is a
  // std::variant<int, float, std::string>
  Value evaluate(Expression expr);
  //@brief Execute running through the given AST
  //@param nodes A vector of nodes from structure Node obtained with the Parser.
  void execute(std::vector<Node> nodes);

  //@brief Returns the evaluate of the conditional
  Value execute_if(IfDeclNode node);
};
#endif
