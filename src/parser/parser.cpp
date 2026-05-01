#include "parser.hpp"
#include "../util/log.hpp"
#include <stdexcept>
#include <string>
#include <vector>

std::vector<Token> Parser::get_token_list() { return Parser::token_list; }

int Parser::get_token_index() { return Parser::token_index; }
void Parser::set_token_index(int i) { Parser::token_index = i; }
Token Parser::now() { return get_token_list()[get_token_index()]; }
Token Parser::peek() {
  if (get_token_index() + 1 < (int)get_token_list().size())
    return get_token_list()[get_token_index() + 1];
  return get_token_list()[get_token_index()];
}
void Parser::advance() {
  if (get_token_index() < (int)get_token_list().size() - 1)
    set_token_index(get_token_index() + 1);
}

std::string Parser::save_token_value(Token t) { return t.value; }

bool Parser::compare_declaration(TOKEN_TYPES type, std::string value) {
  return now().type == type && now().value == value;
}

void Parser::debug_tree(const std::vector<Node> &nodes) {
  Logger::log(Logger::INFO, Logger::PARSER,
              "AST with " + std::to_string(nodes.size()) + " nodes");
  for (int i = 0; i < nodes.size(); i++) {
    Logger::log(Logger::INFO, Logger::PARSER, "node " + std::to_string(i));
  }
}
TransitionDeclNode Parser::parse_TransitionDecl() {

  TransitionDeclNode node;
  std::string property = now().value;
  /* from keyword */
  advance();
  /* from number */
  advance();

  float from = std::stof(now().value);

  /* to keyword */
  advance();

  /* to number */
  advance();
  float to = std::stof(now().value);

  /* easing keyword */
  advance();

  /* easing type */
  std::string easing = now().value;
  advance();
  advance();
  node = TransitionDeclNode{property, from, to, easing};

  return node;
}

ShaderDeclNode Parser::parse_ShaderDecl() {
  /* shader keyword */
  advance();

  std::string identifier_name = save_token_value(now());

  /* L BRACE */
  advance();
  std::string final_glsl;

  /* depth system, it may stop in wrong brackets */
  int depth = 1;
  while (depth > 0) {
    if (now().type == TOKEN_TYPES::LBRACE)
      depth++;
    if (now().type == TOKEN_TYPES::RBRACE)
      depth--;
    if (depth > 0)
      final_glsl.append(now().value);
    advance();
  }
  return ShaderDeclNode{identifier_name, final_glsl};
}

AnimateDeclNode Parser::parse_AnimateDecl() {

  advance();
  advance();

  std::vector<TransitionDeclNode> p;
  while (now().type != TOKEN_TYPES::RBRACE) {
    p.push_back(parse_TransitionDecl());
  }

  return AnimateDeclNode{p};
}

Expression Parser::parse_FnCallExprNode(std::string name) {
  /* ( */
  advance();

  std::vector<Expression> args;
  while (now().type != TOKEN_TYPES::RPARENS) {
    args.push_back(parse_expression());

    if (now().type == TOKEN_TYPES::COMMA) {
      advance();
    }
  }

  /* ) */
  advance();

  return Expression{FnCallExprNode{name, args}};
}

Expression Parser::parse_primary() {

  if (now().type == TOKEN_TYPES::OPERATOR && now().value == "-") {
    advance();
    float val = std::stof(now().value);
    advance();
    return Expression{NumberExprNode{-val}};
  }
  if (now().type == TOKEN_TYPES::NUMBER) {
    float val = std::stof(now().value);
    advance();
    return Expression{NumberExprNode{val}};
  } else if (now().type == TOKEN_TYPES::IDENT) {
    std::string name = now().value;
    advance();

    if (now().type == TOKEN_TYPES::LPARENS) {
      return parse_FnCallExprNode(name);
    }
    return Expression{IdentExprNode{name}};
  } else if (now().type == TOKEN_TYPES::STRING) {
    std::string val = now().value;
    advance();
    return Expression{StringExprNode{val}};
  }
  throw std::runtime_error("expected expression at line " +
                           std::to_string(now().line));
}

Expression Parser::parse_term() {
  Expression left = parse_primary();

  while (now().type == TOKEN_TYPES::OPERATOR &&
         (now().value == "*" || now().value == "/")) {
    std::string op = now().value;
    advance();
    Expression right = parse_primary();
    left = Expression{BinaryExprNode{std::make_shared<Expression>(left), op,
                                     std::make_shared<Expression>(right)}};
  }

  return left;
}

Expression Parser::parse_expression() {
  Expression left = parse_term();

  while (now().type == TOKEN_TYPES::OPERATOR &&
         (now().value == "+" || now().value == "-")) {
    std::string op = now().value;
    advance();
    Expression right = parse_term();
    left = Expression{BinaryExprNode{std::make_shared<Expression>(left), op,
                                     std::make_shared<Expression>(right)}};
  }

  return left;
}
/* declaration of parser's things*/
VarDeclNode Parser::parse_VarDecl() {
  /* starting without the "var" keyword!! */
  advance();

  std::string identifier_name = save_token_value(now());

  /* "equal" */
  advance();

  advance();

  Expression value = parse_expression();
  return VarDeclNode{identifier_name, value};
}

SceneDeclNode Parser::parse_SceneDecl() {
  /* scene keyword */
  advance();

  std::string identifier_name = save_token_value(now());
  advance();
  /* r brackets */
  advance();

  /* body */
  std::vector<std::shared_ptr<Node>> body;
  while (now().type != TOKEN_TYPES::RBRACE) {
    if (compare_declaration(TOKEN_TYPES::KEYWORD, "shader")) {
      body.push_back(std::make_shared<Node>(Node{parse_ShaderDecl()}));
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "animate")) {
      body.push_back(std::make_shared<Node>(Node{parse_AnimateDecl()}));
    } else {
      advance();
    }
  }

  advance();
  return SceneDeclNode{identifier_name, body};
}

FnDeclNode Parser::parse_FnDecl() {

  /* fn keyword */
  advance();

  std::string func_name = save_token_value(now());

  /* name */
  advance();

  /* first param or ) */
  advance();
  std::vector<std::string> params;
  while (now().type != TOKEN_TYPES::RPARENS) {
    if (now().type == TOKEN_TYPES::IDENT) {
      params.push_back(now().value);
    }
    advance();
  }

  /* ) */
  advance();

  /* {... body */
  std::vector<Node> body;

  /* lil fallback to 0 */
  Expression return_expr{NumberExprNode{0.0f}};

  while (now().type != TOKEN_TYPES::RBRACE) {
    if (compare_declaration(TOKEN_TYPES::KEYWORD, "var")) {
      body.push_back(Node{parse_VarDecl()});
    } else if (now().type == TOKEN_TYPES::IDENT &&
               peek().type != TOKEN_TYPES::LPARENS) {

      /* implicit return */
      return_expr = parse_expression();
    } else if (now().type == TOKEN_TYPES::IDENT &&
               peek().type == TOKEN_TYPES::LPARENS) {
      /* TODO: func call by statement */
      return_expr = parse_expression();
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "if")) {
      body.push_back(Node{parse_IfDeclNode()});
    } else {
      advance();
    }
  }

  /* } */
  advance();

  return FnDeclNode{func_name, params, body, return_expr};
}

IfDeclNode Parser::parse_IfDeclNode() {
  /* if keyword */
  advance();
  /* ( */
  advance();

  Expression conditional = parse_expression();

  /* ) */
  advance();
  /* { */
  advance();

  std::vector<Node> then_body;
  Expression then_expr{NumberExprNode{0.0f}};

  while (now().type != TOKEN_TYPES::RBRACE) {
    if (compare_declaration(TOKEN_TYPES::KEYWORD, "var")) {
      then_body.push_back(Node{parse_VarDecl()});
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "if")) {
      then_body.push_back(Node{parse_IfDeclNode()});
    } else if (now().type == TOKEN_TYPES::IDENT ||
               now().type == TOKEN_TYPES::NUMBER) {
      then_expr = parse_expression();
    } else {
      advance();
    }
  }

  /* } */
  advance();

  std::vector<Node> else_body;
  Expression else_expr{NumberExprNode{0.0f}};
  if (compare_declaration(TOKEN_TYPES::KEYWORD, "else")) {
    /* else keyword */
    advance();
    /* { */
    advance();

    while (now().type != TOKEN_TYPES::RBRACE) {
      if (compare_declaration(TOKEN_TYPES::KEYWORD, "var")) {
        else_body.push_back(Node{parse_VarDecl()});
      } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "if")) {
        else_body.push_back(Node{parse_IfDeclNode()});
      } else if (now().type == TOKEN_TYPES::IDENT ||
                 now().type == TOKEN_TYPES::NUMBER) {
        else_expr = parse_expression();
      } else {
        advance();
      }
    }
    /* } */
    advance();
  }

  return IfDeclNode{conditional, then_body, then_expr, else_body, else_expr};
}

std::vector<Node> Parser::parse() {
  Logger::log(Logger::INFO, Logger::PARSER, "--- parse initiating ---");
  /* easier life over memory!!11!! */
  std::vector<Token> token_list = get_token_list();

  /* checking all of the tokens and calling the right functions to deal with
   * them */
  std::vector<Node> nodes;
  while (token_list.size() - 1 > (size_t)get_token_index()) {

    /* if the current token is a keyword and is a "var" keyword... */
    if (compare_declaration(TOKEN_TYPES::KEYWORD, "var")) {

      /* parses it with the VarDecl function and pushes into the final node
       * vector */
      nodes.push_back(Node{parse_VarDecl()});
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "scene")) {

      /* SceneDecl is more complex, it can call other "parse" functions, like
       * parse_TransitionDecl */
      nodes.push_back(Node{parse_SceneDecl()});
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "fn")) {

      /* parses it with the FnDecl function and pushes into the final node
       * vector*/
      nodes.push_back(Node{parse_FnDecl()});
    } else {

      advance();
    }

    // if (token_index < token_list.size() - 1)
    //   advance();
  }

  Logger::log(Logger::INFO, Logger::PARSER, "parse complete");
  return nodes;
}
