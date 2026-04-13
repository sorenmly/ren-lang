#include "interpreter.hpp"
#include "../util/log.hpp"
#include <stdexcept>
#include <unordered_map>

std::unordered_map<std::string, Expression> Interpreter::get_node_map() {
  return node_map;
}
Value Interpreter::evaluate(Expression expr) {
  return std::visit(
      [&](auto &&e) -> Value {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, NumberExprNode>) {
          return Value{e.value};
        } else if constexpr (std::is_same_v<T, IdentExprNode>) {
          if (node_map.count(e.name)) {
            return evaluate(node_map[e.name]);
          }
          throw std::runtime_error("undefined variable: " + e.name);
        } else if constexpr (std::is_same_v<T, BinaryExprNode>) {
          Value left = evaluate(*e.left);
          Value right = evaluate(*e.right);

          if (std::holds_alternative<float>(left) &&
              std::holds_alternative<float>(right)) {
            float l = std::get<float>(left);
            float r = std::get<float>(right);
            if (e.op == "+")
              return Value{l + r};
            if (e.op == "-")
              return Value{l - r};
            if (e.op == "*")
              return Value{l * r};
            if (e.op == "/")
              return Value{l / r};
          }
          throw std::runtime_error(
              "cannot apply operator to non-numeric values");
        } else if constexpr (std::is_same_v<T, StringExprNode>) {

          return Value{e.value};
        }
        return Value{0.0f};
      },
      expr.value);
}

void Interpreter::execute(std::vector<Node> nodes) {
  /* dear c++... */

  /* explaining so i don't get lost latter */
  /* 1. looping through each node in our AST */
  for (auto &node : nodes) {
    /* std::visit checks our Node struct type (it's a variant, it can be alot
     * of things!!) */
    std::visit(
        [&](auto &&n) {
          /* "n" is our type */

          /* 2. picking our type without any qualificators (&, &&...) */
          using T = std::decay_t<decltype(n)>;

          /* 3. "if our type n is VarDeclNode, please store it in the
           * unordered map" */
          if constexpr (std::is_same_v<T, VarDeclNode>) {
            Logger::log(Logger::INFO, Logger::INTERPRETER,
                        "allocating variable declaration in map");
            node_map[n.name] = n.value;
          } else if constexpr (std::is_same_v<T, SceneDeclNode>) {
            Logger::log(Logger::INFO, Logger::INTERPRETER, "executing scene");

            /* executing body, it needs to be an std::visit cuz Node is a
             * variant */
            for (auto &child : n.body) {
              std::visit(
                  [&](auto &&c) {
                    using CT = std::decay_t<decltype(c)>;

                    if constexpr (std::is_same_v<CT, ShaderDeclNode>) {
                      Logger::log(Logger::INFO, Logger::INTERPRETER,
                                  "found shader: " + c.type);
                      // TODO: compile glsl
                    } else if constexpr (std::is_same_v<CT, AnimateDeclNode>) {
                      Logger::log(Logger::INFO, Logger::INTERPRETER,
                                  "found animate block");
                      // TODO: process transitions
                    }
                  },
                  child->value);
            }
            /* TODO: renderer */
          }
        },
        node.value);
  }
}
