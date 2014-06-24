#pragma once

#include <cstdint>

namespace calc {

	/** Type of an expression node  */
	enum ExprType {
		ADD,
		SUB,
		MUL,
		DIV,
		VAL,
		VAR
	};

	/** The node of an expression. Either a value or an operation. */
	struct Expr {
		ExprType type;
		uint32_t val;
		Expr* left;
		Expr* right;

		/** Create a new value or variable node. */
		Expr(ExprType type, uint32_t val) : type(type), val(val), left(nullptr), right(nullptr) {}
		/** Create a new operation node. */
		Expr(ExprType type, Expr* left, Expr* right) : type(type), left(left), right(right) {}
	};

}
