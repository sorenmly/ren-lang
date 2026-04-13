#include "lexer.hpp"
#include "../util/log.hpp"
#include <cctype>
#include <iostream>
#include <set>
#include <string>

std::set<std::string> KEYWORD_TYPES = {"var",  "scene", "shader", "animate",
                                       "from", "to",    "easing"};

int Lexer::get_index() { return Lexer::index; }
std::string Lexer::get_src() { return Lexer::src; }
void Lexer::set_index(int i) { Lexer::index = i; }
int Lexer::get_line() { return Lexer::line; }
int Lexer::get_column() { return Lexer::column; }
void Lexer::set_line(int i) { Lexer::line = i; }
void Lexer::set_column(int i) { Lexer::column = i; }

char Lexer::now() { return Lexer::get_src()[Lexer::get_index()]; }
char Lexer::peek(int i) { return Lexer::get_src()[Lexer::get_index() + i]; }
void Lexer::advance() {
  Lexer::set_index(Lexer::get_index() + 1);
  Lexer::set_column(Lexer::get_column() + 1);
}

std::string tok_types_to_string(TOKEN_TYPES src) {
  switch (src) {
  case TOKEN_TYPES::VAR:
    return "VAR";
  case TOKEN_TYPES::KEYWORD:
    return "KEYWORD";
  case TOKEN_TYPES::LBRACE:
    return "LBRACE";
  case TOKEN_TYPES::RBRACE:
    return "RBRACE";
  case TOKEN_TYPES::SEMICOLON:
    return "SEMICOLON";
  case TOKEN_TYPES::NUMBER:
    return "NUMBER";
  case TOKEN_TYPES::STRING:
    return "STRING";
  case TOKEN_TYPES::IDENT:
    return "IDENT";
  case TOKEN_TYPES::LPARENS:
    return "LPARENS";
  case TOKEN_TYPES::RPARENS:
    return "RPARENS";
  case TOKEN_TYPES::EQUAL:
    return "EQUAL";
  case TOKEN_TYPES::OPERATOR:
    return "OPERATOR";
  default:
    return "UNKNOWN";
  }
}

void Lexer::debug_tokens(std::vector<Token> l) {
  Logger::log(Logger::INFO, Logger::LEXER, "printing found tokens.");
  for (int i = 0; i < l.size(); i++) {
    Logger::log(Logger::INFO, Logger::LEXER,
                "type: " + tok_types_to_string(l[i].type) + "; value: [" +
                    l[i].value + "]; line, column: (" +
                    std::to_string(l[i].line) + ", " +
                    std::to_string(l[i].column) + ")");
  }
}

void Lexer::read_comments() {
  while (now() != '\n' && index < src.size()) {
    advance();
  }
}

Token Lexer::read_number() {
  std::string n(1, now());

  while (std::isdigit(peek())) {
    advance();
    n.append(std::string(1, now()));
  }
  if (peek() == '.' && std::isdigit(peek(2))) {
    advance();
    n.append(std::string(1, now()));
    while (std::isdigit(peek())) {
      advance();
      n.append(std::string(1, now()));
    }
  }
  return Token{TOKEN_TYPES::NUMBER, n, Lexer::get_line(), Lexer::get_column()};
}

Token Lexer::read_strings() {
  std::string n(1, now());

  advance();
  while (now() != '"') {
    n.append(std::string(1, now()));
    advance();
  }
  n.append(std::string(1, now()));
  return Token{TOKEN_TYPES::STRING, n, get_line(), get_column()};
}

Token Lexer::read_division() {
  return Token{TOKEN_TYPES::OPERATOR, "/", get_line(), get_column()};
}

Token Lexer::read_alpha() {
  std::string n(1, now());

  while (std::isalpha(peek()) || std::isdigit(peek()) || peek() == '_') {
    advance();
    n.append(std::string(1, now()));
  }

  if (KEYWORD_TYPES.count(n)) {
    return Token{TOKEN_TYPES::KEYWORD, n, get_line(), get_column()};
  } else {
    return Token{TOKEN_TYPES::IDENT, n, get_line(), get_column()};
  }
}

std::vector<Token> Lexer::lexer_process() {
  Logger::log(Logger::INFO, Logger::LEXER, "--- starting tokenization ---");
  std::string src = Lexer::get_src();

  std::vector<Token> tokens;

  while (src.size() > index) {
    if (std::isdigit(now())) {
      tokens.push_back(read_number());
      advance();
    } else if (std::isalpha(now())) {

      tokens.push_back(read_alpha());

      advance();
    } else if (now() == '=') {
      tokens.push_back(
          Token{TOKEN_TYPES::EQUAL, "=", get_line(), get_column()});
      advance();

    } else if (std::isspace(now())) {
      if (now() == '\n') {
        Lexer::set_line(Lexer::get_line() + 1);
        Lexer::set_column(1);
      }

      advance();
    } else if (now() == '}') {
      tokens.push_back(Token{TOKEN_TYPES::RBRACE, std::string(1, now()),
                             get_line(), get_column()});
      advance();
    } else if (now() == '{') {
      tokens.push_back(Token{TOKEN_TYPES::LBRACE, std::string(1, now()),
                             get_line(), get_column()});
      advance();
    } else if (now() == '"') {
      tokens.push_back(read_strings());

      advance();
    } else if (now() == '/') {
      if (peek() == '/') {
        read_comments();
      } else {
        tokens.push_back(read_division());
        advance();
      }
    } else if (now() == '+') {
      tokens.push_back(
          Token{TOKEN_TYPES::OPERATOR, "+", get_line(), get_column()});
      advance();
    } else if (now() == '-') {
      tokens.push_back(
          Token{TOKEN_TYPES::OPERATOR, "-", get_line(), get_column()});
      advance();
    } else if (now() == '*') {
      tokens.push_back(
          Token{TOKEN_TYPES::OPERATOR, "*", get_line(), get_column()});
      advance();
    } else {
      Logger::log(Logger::WARN, Logger::LEXER,
                  "unknown token (" + std::string(1, now()) + ") in line (" +
                      std::to_string(get_line()) + ", " +
                      std::to_string(get_column()) + ")");
      tokens.push_back(Token{TOKEN_TYPES::UNKNOWN, std::string(1, now()),
                             get_line(), get_column()});
      advance();
    }
  }
  Logger::log(Logger::INFO, Logger::LEXER, "tokenization complete!");
  return tokens;
}
