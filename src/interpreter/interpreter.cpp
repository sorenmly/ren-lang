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
            if (e.op == ">")
              return Value{l > r ? 1.0f : 0.0f};
            if (e.op == "<")
              return Value{l < r ? 1.0f : 0.0f};
            if (e.op == ">=")
              return Value{l >= r ? 1.0f : 0.0f};
            if (e.op == "<=")
              return Value{l <= r ? 1.0f : 0.0f};
            if (e.op == "==")
              return Value{l == r ? 1.0f : 0.0f};
            if (e.op == "!=")
              return Value{l != r ? 1.0f : 0.0f};
          }
          throw std::runtime_error(
              "cannot apply operator to non-numeric values");

        } else if constexpr (std::is_same_v<T, StringExprNode>) {
          return Value{e.value};

        } else if constexpr (std::is_same_v<T, FnCallExprNode>) {
          if (!fn_map.count(e.name)) {
            throw std::runtime_error("undefined function: " + e.name);
          }

          FnDeclNode fn = fn_map[e.name];

          if (e.args.size() != fn.params.size()) {
            throw std::runtime_error("wrong number of arguments for: " +
                                     e.name);
          }

          /* evaluate args before entering the new scope */
          std::vector<Expression> evaluated_args;
          for (int i = 0; i < (int)e.args.size(); i++) {
            Value v = evaluate(e.args[i]);
            std::visit(
                [&](auto &&val) {
                  using VT = std::decay_t<decltype(val)>;
                  if constexpr (std::is_same_v<VT, float>)
                    evaluated_args.push_back(Expression{NumberExprNode{val}});
                  else if constexpr (std::is_same_v<VT, std::string>)
                    evaluated_args.push_back(Expression{StringExprNode{val}});
                  else
                    evaluated_args.push_back(
                        Expression{NumberExprNode{(float)val}});
                },
                v);
          }

          /* save current scope, inject params, execute, restore */
          auto saved_scope = node_map;
          for (int i = 0; i < (int)fn.params.size(); i++) {
            node_map[fn.params[i]] = evaluated_args[i];
          }

          execute(fn.body);
          Value result = evaluate(fn.return_expr);

          node_map = saved_scope;
          return result;
        }

        return Value{0.0f};
      },
      expr.value);
}

void Interpreter::execute(std::vector<Node> nodes) {
  for (auto &node : nodes) {
    std::visit(
        [&](auto &&n) {
          using T = std::decay_t<decltype(n)>;

          if constexpr (std::is_same_v<T, VarDeclNode>) {
            Logger::log(Logger::INFO, Logger::INTERPRETER,
                        "allocating variable declaration in map");
            /* evaluate eagerly so we store a resolved value, not a raw
             * expression */
            Value resolved = evaluate(n.value);
            std::visit(
                [&](auto &&v) {
                  using VT = std::decay_t<decltype(v)>;
                  if constexpr (std::is_same_v<VT, float>)
                    node_map[n.name] = Expression{NumberExprNode{v}};
                  else if constexpr (std::is_same_v<VT, std::string>)
                    node_map[n.name] = Expression{StringExprNode{v}};
                  else
                    node_map[n.name] = Expression{NumberExprNode{(float)v}};
                },
                resolved);

          } else if constexpr (std::is_same_v<T, SceneDeclNode>) {
            Logger::log(Logger::INFO, Logger::INTERPRETER, "executing scene");
            for (auto &child : n.body) {
              std::visit(
                  [&](auto &&c) {
                    using CT = std::decay_t<decltype(c)>;
                    if constexpr (std::is_same_v<CT, ShaderDeclNode>) {
                      Logger::log(Logger::INFO, Logger::INTERPRETER,
                                  "found shader: " + c.type);
                      /* TODO: compile glsl */
                    } else if constexpr (std::is_same_v<CT, AnimateDeclNode>) {
                      Logger::log(Logger::INFO, Logger::INTERPRETER,
                                  "found animate block");
                      /* TODO: process transitions */
                    }
                  },
                  child->value);
            }
            /* TODO: renderer */

          } else if constexpr (std::is_same_v<T, FnDeclNode>) {
            Logger::log(Logger::INFO, Logger::INTERPRETER,
                        "registering function: " + n.name);
            fn_map[n.name] = n;
          } else if constexpr (std::is_same_v<T, IfDeclNode>) {
            execute_if(n);
          }
        },
        node.value);
  }
}

Value Interpreter::execute_if(IfDeclNode node) {
  Value cond = evaluate(node.condition);

  bool is_true = false;
  std::visit(
      [&](auto &&v) {
        using VT = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<VT, float>)
          is_true = v != 0.0f;
      },
      cond);

  Logger::log(Logger::INFO, Logger::INTERPRETER,
              "if condition result: " +
                  std::string(is_true ? "true" : "false"));

  Value result{0.0f};
  if (is_true) {
    execute(node.then_body);
    result = evaluate(node.then_expr);
  } else {
    execute(node.else_body);
    result = evaluate(node.else_expr);
  }

  has_return = true;
  return_value = result;

  std::visit(
      [](auto &&v) {
        using VT = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<VT, float>)
          Logger::log(Logger::INFO, Logger::INTERPRETER,
                      "if return value: " + std::to_string(v));
      },
      return_value);

  return result;
}
