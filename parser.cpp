#include <cstdlib>
#include <iostream>
#include "parser.h"

namespace calc {

  /**
   * Reads the next character and moves the string pointer.
   * 
   * @param str A pointer to the character array.
   * @return The last character that has been read.
   */
  static char readNext(char* &str);

  /**
   * Parses a term consisting of multiple factors.
   *
   * @param str A pointer o the character array.
   * @return The parsed term.
   */
  static Expr* parseTerm(char* &str);

  /**
   * Parses a factor which is either a value or an expression in parentheses.
   *
   * @param str A pointer to the character array.
   * @return The parsed factor.
   */
  static Expr* parseFactor(char* &str);

  /**
   * Parses a factor which is either a value or an expression in parentheses.
   *
   * @param str A pointer to the character array.
   * @return The parsed factor.
   */
  static uint64_t parseValue(char* &str);

  Expr* parseExpression(char* &str) {
    Expr* expression = parseTerm(str);

    while (true) {
      switch (readNext(str)) {
        case '+':
          expression = new Expr(ADD, expression, parseTerm(str));
          break;
        case '-':
          expression = new Expr(SUB, expression, parseTerm(str));
          break;
        default:
          --str;
          return expression;
      }
    }
  }

  static char readNext(char* &str) {
    while (*str == ' ') ++str;
    return *str++;
  }

  static Expr* parseTerm(char* &str) {
    Expr* term = parseFactor(str);

    while (true) {
      switch(readNext(str)) {
        case '*':
          term = new Expr(MUL, term, parseFactor(str));
          break;
        case '/':
          term = new Expr(DIV, term, parseFactor(str));
          break;
        default:
          --str;
          return term;
      }
    }
  }

  static Expr* parseFactor(char* &str) {
    char c = readNext(str);

    if (c == '(') {
      Expr* expression = parseExpression(str);
      if (readNext(str) != ')') {
        std::cerr << "Missing ')'." << std::endl;
        exit(1);
      }
      return expression;
    } else if (c == '_' || c == '#' || c == '$') {
      int64_t val = parseValue(str);
      return new Expr(VAR, val);
    } else if (c == '-') {
      int64_t val = parseValue(str);
      return new Expr(VAL, -val);
    } else if (c >= '0' && c <= '9') {
      int64_t val = parseValue(--str);
      return new Expr(VAL, val);
    } else {
      std::cerr << "Unexpected token '" << c << "'." << std::endl;
      exit(1);
    }
  }

  static uint64_t parseValue(char* &str) {
    uint64_t val = 0;

    char c = readNext(str);
    while (c >= '0' && c <= '9') {
      val = 10 * val + (c - '0');
      c = readNext(str);
    }

    --str;
    return val;
  }

}
