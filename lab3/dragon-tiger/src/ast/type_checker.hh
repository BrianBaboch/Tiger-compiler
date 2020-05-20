#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include "nodes.hh"
#include "../utils/errors.hh"
#include <vector>

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor{


public:
  void visit(IntegerLiteral &literal) override;
  void visit(StringLiteral &literal) override;
  void visit(Sequence &seq) override;
  void visit(IfThenElse &ite) override;
  void visit(Let &let) override;
  void visit(VarDecl &decl) override;
  void visit(BinaryOperator &binOp) override;
  void visit(Identifier &id) override;
  void visit(Assign &assign) override;
  void visit(WhileLoop &loop) override;
  void visit(ForLoop &loop) override;
  void visit(Break &b) override;
  void visit(FunDecl &decl) override;
  void visit(FunCall &decl) override;

};

} // namespace type_checker
} // namespace ast

#endif // TYPE_CHECKER_HH
