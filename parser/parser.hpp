#ifndef PARSER_HPP
#define PARSER_HPP

#include "../lexer/lexer.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

using Value = std::variant<int, float, std::string>;
struct Node;
struct StructDeclNode {
  std::string name;
};
struct SceneDeclNode {
  std::string name;
  /* shared_ptr always has a defined memory value, so we can create the variant!
   */
  std::vector<std::shared_ptr<Node>> body;
};
struct TransitionDeclNode {
  std::string property;
  float from;
  float to;
  std::string easing;
};
struct AnimateDeclNode {
  std::vector<TransitionDeclNode> body;
};
struct ShaderDeclNode {
  std::string type;
  std::string src;
};
struct VarDeclNode {
  std::string name;
  Value value;
};

struct Node {
  std::variant<VarDeclNode, SceneDeclNode, ShaderDeclNode, AnimateDeclNode>
      value;
};

/* it's going to take care of the AST and the interpreter */
class Parser {
private:
  std::vector<Token> token_list;
  int token_index;

public:
  Parser(std::vector<Token> token_list)
      : token_list(token_list), token_index(0) {}

  //@brief Returns the current token list from the parser.
  //@returns An vector of Token's (Token class from lexer).
  std::vector<Token> get_token_list();

  //@brief Returns the current token index. Used to get token_list current item.
  //@retuns Integer containing the index.
  int get_token_index();

  //@brief Sets the token_index to parameter "i"
  //@param i The index that it's going to be set.
  void set_token_index(int i);

  //@brief Returns the current token.
  //@returns The current where Token is from the class Token in the lexer.
  Token now();

  //@brief Peeks the nerxt token without consuming it.
  //@returns Next Token, where Token is from the class Token in the lexer.
  Token peek();

  //@brief Consumes the next token and advances the token_index value.
  void advance();

  //@brief Parses the variable declaration. It implements and follows the rules
  // in the "lang_constr.md".
  //@returns The structure VarDeclNode containing the node already made.
  VarDeclNode parse_VarDecl();

  //@brief Parses the scene declaration. It implements and follows the rules in
  // the "lang_constr.md".
  //@returns The structure SceneDeclNode containing the node already made.
  SceneDeclNode parse_SceneDecl();

  //@brief Parses the transition declaration. It implements and follows the
  // rules in the "lang_constr.md".
  //@returns The structure TransitionDeclNode containing the node already made.
  TransitionDeclNode parse_TransitionDecl();

  //@brief Parses the animate declaration. It implements and follows the rules
  // in the "lang_constr.md".
  //@returns The structure AnimateDeclNode containing the node already made.
  AnimateDeclNode parse_AnimateDecl();

  //@brief Parses the shader declaration. It implements and follows the rules in
  // the "lang_constr.md".
  //@returns The structure ShaderDeclNode containing the node already made.
  ShaderDeclNode parse_ShaderDecl();

  //@brief Picks up an Token t and returns the value string.
  //@param t The token, where Token is the class Token from the lexer.
  std::string save_token_value(Token t);

  //@brief Comparates the current token with the given Token parameters.
  //@param type The type of the token to be compared to.
  //@param value The value of the token to be compared to.
  bool compare_declaration(TOKEN_TYPES type, std::string value);

  //@brief Prints the formed AST for this parser.
  void debug_tree(const std::vector<Node> &nodes);

  std::vector<Node> parse();
};
#endif
