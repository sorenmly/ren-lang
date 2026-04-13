#ifndef LEXER_CPP
#define LEXER_CPP

#include <string>
#include <vector>

enum class TOKEN_TYPES {

  VAR,
  KEYWORD,
  LBRACE,
  RBRACE,
  SEMICOLON,
  NUMBER,
  STRING,
  IDENT,
  LPARENS,
  RPARENS,
  EQUAL,
  OPERATOR,
  UNKNOWN
};

class Token {
public:
  TOKEN_TYPES type;
  std::string value;
  int line;
  int column;

  Token(TOKEN_TYPES type, std::string value, int line, int column)
      : type(type), value(value), line(line), column(column) {}
};

class Lexer {
private:
  std::string src;
  int index;
  int line;
  int column;

public:
  Lexer(std::string src) : src(src), index(0), line(1), column(1) {}
  //@brief Final function for the lexer. Processes the given source and returns
  // a std::vector<Token> of tokens.
  //@param src The code source that is going to be processed.
  //@return Returns an vector (std::vector) of tokens.
  std::vector<Token> lexer_process();

  //@brief Peek next character without consuming.
  //@returns Char containing the next character
  char peek(int i = 1);

  //@brief Consumes next character.
  void advance();

  //@brief Get current index char.
  //@returns Integer containing index
  int get_index();

  //@brief Get current line.
  //@returns Integer containing line
  int get_line();

  //@brief Get current column.
  //@return Integer containing column
  int get_column();

  //@brief Sets current line.
  void set_line(int i);

  //@brief Sets current column.
  void set_column(int i);

  //@brief Sets the index to i.
  //@param i The value for the index.
  void set_index(int i);

  //@brief Gets current source code.
  //@returns Current source code.
  std::string get_src();

  //@brief Gets current character based on index.
  //@returns The current character in char.
  char now();

  //@brief Used in lexer to correctly read a number. Not mean't for
  // further use.
  //@returns Token structure containing the full token.
  Token read_number();

  //@brief Used in lexer to correctly read a alphanumeric glyph. Not
  // mean't for further use.
  //@returns Token structure containing the full token.
  Token read_alpha();

  //@brief Used in lexer to correctly read a string. Not mean't for
  // further use.
  //@returns Token structure containing the full token.
  Token read_strings();

  //@brief Used i lexer to correctly read comments. Not mean't for further use
  //@returns Token structure containing the full token.
  void read_comments();

  //@brief Used in lexer to correctly read a division. Not mean't for
  // further use.
  //@returns Token structure containing the full token.
  Token read_division();

  //@brief Prints the tokens gathered from the lexer_process() function.
  //@param l A vector of tokens usually gattered with lexer_process().
  void debug_tokens(std::vector<Token> l);
};

#endif
