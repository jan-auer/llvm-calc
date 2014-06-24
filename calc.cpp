#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include "expr.h"
#include "parser.h"

using namespace llvm;
using namespace calc;

static Function* createFunction(Module* module, const std::string& name, Type* returnType, int argCount = 0, Type* argType = nullptr) {
  std::vector<Type*> params(argCount, argType);
  FunctionType* funcType = FunctionType::get(returnType, params, false);
  return cast<Function>(module->getOrInsertFunction(name, funcType));
}

static std::vector<Value*> extractParameters(Function* function) {
  std::vector<Value*> params;
  for (auto args = function->arg_begin(); args != function->arg_end();)
    params.push_back(args++);

  return params;
}

static Value* compileExpression(LLVMContext& context, const Expr* expression, const std::vector<Value*>& params, BasicBlock* block) {
  if (expression->type == VAL)
    return ConstantInt::get(Type::getInt64Ty(context), expression->val);

  if (expression->type == VAR)
    return params[expression->val];

  Value* lhs = compileExpression(context, expression->left, params, block);
  Value* rhs = compileExpression(context, expression->right, params, block);

  switch (expression->type) {
    case ADD:
      return BinaryOperator::CreateAdd(lhs, rhs, "ADD", block);
    case SUB:
      return BinaryOperator::CreateSub(lhs, rhs, "Sub", block);
    case MUL:
      return BinaryOperator::CreateMul(lhs, rhs, "Mul", block);
    case DIV:
      return BinaryOperator::CreateSDiv(lhs, rhs, "Div", block);
    default:
      // cannot happen
      return nullptr;
  }
}

static int countVars(const Expr* expression) {
  if (expression->type == VAR)
    return expression->val + 1;

  if (expression->type == VAL)
    return 0;

  return std::max(countVars(expression->left), countVars(expression->right));
}

static Function* compileFunction(LLVMContext& context, Module* module, const std::string& name, const Expr* expression) {
  Type* argType = Type::getInt64Ty(context);
  Type* returnType = Type::getInt64Ty(context);

  Function* func = createFunction(module, name, returnType, countVars(expression), argType);
  BasicBlock* block = BasicBlock::Create(context, "EntryBlock", func);
  Value* result = compileExpression(context, expression, extractParameters(func), block);
  ReturnInst::Create(context, result, block);

  return func;
}

static ExecutionEngine* createEngine(Module* module, EngineKind::Kind kind = EngineKind::JIT) {
  std::string errStr;
  ExecutionEngine* engine = EngineBuilder(module)
      .setErrorStr(&errStr)
      .setEngineKind(kind)
      .create();

  if (!engine) {
    errs() << "Failed to construct ExecutionEngine: " << errStr << "\n";
    exit(1);
  }

  errs() << "verifying... ";
  if (verifyModule(*module)) {
    errs() << "Error constructing function!\n";
    exit(1);
  }
  errs() << "OK\n";

  errs() << "We just constructed this LLVM module:\n\n---------\n" << *module;
  errs() << "---------\nstarting calc with " 
         << (kind==EngineKind::Interpreter ? "interpreter" 
          : (kind==EngineKind::JIT) ? "JIT" 
          : "???" )
         << " ...\n\n";

  return engine;
}

static std::vector<GenericValue> parseArguments(int argc, char** argv) {
  std::vector<GenericValue> args(argc);
  for (int i = 0; i < argc; ++i)
    args[i].IntVal = APInt(64, atol(argv[i]));

  return args;
}

int main(int argc, char **argv) {
  InitializeNativeTarget();

  // Create a module containing the `calc` function
  LLVMContext context;
  OwningPtr<Module> module(new Module("test", context));

  // Create the expression and compile it
  Expr* expression = parseExpression(argv[1]);
  Function* calcFunction = compileFunction(context, module.get(), "calc", expression);

  // Initialize the LLVM JIT engine.
  ExecutionEngine* engine = createEngine(module.get());

  // Compute the result and print it out
  auto calcArgs = parseArguments(argc - 2, argv + 2);
  GenericValue calcValue = engine->runFunction(calcFunction, calcArgs);
  outs() << "Result: " << calcValue.IntVal << "\n";

  return 0;
}
