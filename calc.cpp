#include <cstdint>

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

/** Recursively creates code for an operation. */
static Value* createOperation(LLVMContext& context, Value* params, Expr* expression, BasicBlock* block) {
  if (expression->type == VAL) {
    return ConstantInt::get(Type::getInt64Ty(context), expression->val);
  } else if (expression->type == VAR) {
    Value* i = ConstantInt::get(Type::getInt32Ty(context), expression->val);
    Value* pointer = GetElementPtrInst::Create(params, i, "aptr", block);
    return new LoadInst(pointer, "a", block);
  }

  Value* lhs = createOperation(context, params, expression->left, block);
  Value* rhs = createOperation(context, params, expression->right, block);

  switch (expression->type) {
  	case ADD:
      return BinaryOperator::CreateAdd(lhs, rhs, "ADD", block);
    case SUB:
      return BinaryOperator::CreateSub(lhs, rhs, "Sub", block);
    case MUL:
      return BinaryOperator::CreateMul(lhs, rhs, "Mul", block);
    case DIV:
      return BinaryOperator::CreateSDiv(lhs, rhs, "Div", block);
    case VAL:
    default:
    	return nullptr;
  }
}

/** Creates code for the calc function. */
static Function* createCalcFunction(Module* module, LLVMContext& context, Expr* expression, Type* returnType, Type* argType) {
  Function* func = cast<Function>(module->getOrInsertFunction("calc", returnType, argType, nullptr));
  BasicBlock* block = BasicBlock::Create(context, "EntryBlock", func);
  
  Value* params = func->arg_begin()++;
  Value* result = createOperation(context, params, expression, block);
  ReturnInst::Create(context, result, block);
  return func;
}

int main(int argc, char **argv) {
  InitializeNativeTarget();
  LLVMContext context;

  // Create the expression to compile
  Expr* expression = parseExpression(argv[1]);

  // Create a module containing the `calc` function
  OwningPtr<Module> module(new Module("test", context));
  Type* returnType = Type::getInt64Ty(context);
  Type* argType = PointerType::get(returnType, 0);
  Function* calcFunction = createCalcFunction(module.get(), context, expression, returnType, argType);

  // Initialize the LLVM JIT engine.
  auto kind = EngineKind::JIT;
  std::string errStr;
  ExecutionEngine *engine = EngineBuilder(module.get())
      .setErrorStr(&errStr)
      .setEngineKind(kind)
      .create();

  if (!engine) {
    errs() << argv[0] << ": Failed to construct ExecutionEngine: " << errStr << "\n";
    return 1;
  }

  errs() << "verifying... ";
  if (verifyModule(*module)) {
    errs() << argv[0] << ": Error constructing function!\n";
    return 1;
  }
  errs() << "OK\n";
  errs() << "We just constructed this LLVM module:\n\n---------\n" << *module;
  errs() << "---------\nstarting calc(" << 1 << ") with " 
         << (kind==EngineKind::Interpreter ? "interpreter" 
         	: (kind==EngineKind::JIT) ? "JIT" 
         	: "???" )
         << " ...\n";

  // Take the call parameters and pass them into a pointer of `calc`
  std::vector<uint64_t> args;
  for (int i = 2; i < argc; ++i)
  	args.push_back(atol(argv[i]));
  std::vector<GenericValue> calcArgs(1);
  calcArgs[0].PointerVal = args.data();

  // Compute the result and print it out
  GenericValue calcValue = engine->runFunction(calcFunction, calcArgs);
  outs() << "Result: " << calcValue.IntVal << "\n";

  return 0;
}
