#pragma once

#include "expr.h"

namespace calc {

  /**
   * Parses an expression from the given string.
   * The passed character pointer will be modified!
   *
   * @param str A reference to a character pointer of strings.
   * @return The parsed expression tree.
   */
  Expr* parseExpression(char* &str);

}
