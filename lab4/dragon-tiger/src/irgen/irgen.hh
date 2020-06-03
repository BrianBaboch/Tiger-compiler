#ifndef IRGEN_HH
#define IRGEN_HH

#include <deque>
#include <ostream>

#include "../ast/nodes.hh"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace irgen {
using namespace ast::types;

class IRGenerator : public ConstASTValueVisitor {
  // Hold the core "global" data of LLVM's core infrastructure,
  // including the type and constant uniquing tables.
  llvm::LLVMContext Context;

  // Builder to insert instructions into a basic block.
  llvm::IRBuilder<> Builder;

  // Module generated by this tiger program compilation.
  std::unique_ptr<llvm::Module> Mod;

  // Current function being generated.
  llvm::Function *current_function;
  const FunDecl *current_function_decl;

  // Map variable declarations (including function parameters)
  // to LLVM values. Those values might refer to the current
  // function frame if they are escaping, or to
  // alloca-declared variables if they are not escaping.
  std::map<const VarDecl *, llvm::Value *> allocations;

  // Map loops to their exit blocks, so that early exits can
  // be easily processed.
  std::map<const Loop *, llvm::BasicBlock *> loop_exit_bbs;

  // List of functions to be processed after the current one.
  // This is necessary because in Tiger we might encounter
  // new function definitions while processing a function
  // definition. However, code generation is done one function
  // at a time, so we want to be done with the current function
  // generation before handling the next one.
  std::deque<const FunDecl *> pending_func_bodies;

  // Map escaping variables to their position into the current
  // function frame.
  std::map<const VarDecl *, int> frame_position;

  // Map function declarations to their specific frame types.
  std::map<const FunDecl *, llvm::StructType *> frame_type;

  // Frame of the current function.
  llvm::Value *frame;

  // Generate the LLVM IR code corresponding to a function
  // declaration. If inner function declarations are encountered,
  // they will be stored into pending_func_bodies for later
  // processing.
  void generate_function(const FunDecl &);

  // Return the LLVM type corresponding to a Tiger type.
  llvm::Type *llvm_type(const ast::Type);

  // Generate a new alloca in the entry block of the function
  // for a variable of a given type. A name hint can be given,
  // otherwise automatic naming (%0, %1, etc.) will be used.
  llvm::Value *alloca_in_entry(llvm::Type *Ty, const std::string &name = "");

  // Return the address of a given identifier.
  llvm::Value *address_of(const Identifier &id);

public:
  // Constructor
  IRGenerator();

  // Given the main function declaration, generate the LLVM IR
  // corresponding to the whole program.
  void generate_program(FunDecl *);

  // Print the generated IR.
  void print_ir(std::ostream *);

  // Creates the function frame
  void generate_frame();

  // Finds the right frame
  std::pair<llvm::StructType *, llvm::Value *> frame_up(int levels);

  // Generate the IR corresponding to those AST nodes.
  // Those methods will return either nullptr when no
  // result is expected (a statement for example),
  // or the LLVM value when a result is meaningful.
  virtual llvm::Value *visit(const IntegerLiteral &);
  virtual llvm::Value *visit(const StringLiteral &);
  virtual llvm::Value *visit(const BinaryOperator &);
  virtual llvm::Value *visit(const Sequence &);
  virtual llvm::Value *visit(const Let &);
  virtual llvm::Value *visit(const Identifier &);
  virtual llvm::Value *visit(const IfThenElse &);
  virtual llvm::Value *visit(const VarDecl &);
  virtual llvm::Value *visit(const FunDecl &);
  virtual llvm::Value *visit(const FunCall &);
  virtual llvm::Value *visit(const WhileLoop &);
  virtual llvm::Value *visit(const ForLoop &);
  virtual llvm::Value *visit(const Break &);
  virtual llvm::Value *visit(const Assign &);
};

} // namespace irgen

#endif // _IRGEN_HH
