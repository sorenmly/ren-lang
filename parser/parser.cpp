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
/* declaration of parser's things*/
VarDeclNode Parser::parse_VarDecl() {
  /* starting without the "var" keyword!! */
  advance();

  std::string identifier_name = save_token_value(now());

  /* "equal" */
  advance();

  advance();
  std::variant<std::string, float> var_value;
  switch (now().type) {
  case TOKEN_TYPES::NUMBER:
    var_value = std::stof(save_token_value(now()));
    return VarDeclNode{identifier_name, std::get<float>(var_value)};
    break;

  case TOKEN_TYPES::STRING:
    var_value = save_token_value(now());
    return VarDeclNode{identifier_name, std::get<std::string>(var_value)};
    break;

  default:
    throw std::runtime_error(
        "ren >/ Invalid value in variable declaration at line " +
        std::to_string(now().line) + " and column " +
        std::to_string(now().column));
    break;
  }
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
std::vector<Node> Parser::parse() {
  Logger::log(Logger::INFO, Logger::PARSER, "--- parse initiating ---");
  /* easier life over memory!!11!! */
  std::vector<Token> token_list = get_token_list();

  std::vector<Node> nodes;
  while (token_list.size() - 1 > (size_t)get_token_index()) {
    if (compare_declaration(TOKEN_TYPES::KEYWORD, "var")) {
      nodes.push_back(Node{parse_VarDecl()});
    } else if (compare_declaration(TOKEN_TYPES::KEYWORD, "scene")) {
      nodes.push_back(Node{parse_SceneDecl()});
    } else {
      advance();
    }

    // if (token_index < token_list.size() - 1)
    //   advance();
  }

  Logger::log(Logger::INFO, Logger::PARSER, "parse complete");
  return nodes;
}
