#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include "nodes.hh"
#include "../utils/errors.hh"

namespace ast{
class Evaluator:public ConstASTIntVisitor{
  public:
	  int32_t visit(const class IntegerLiteral & elt) override;
	  int32_t visit(const class StringLiteral & elt) override;
	  int32_t visit(const class BinaryOperator & elt) override;
	  int32_t visit(const class Sequence & elt) override;
	  int32_t visit(const class Let & elt) override;
	  int32_t visit(const class Identifier & elt) override;
	  int32_t visit(const class IfThenElse & elt) override;
	  int32_t visit(const class VarDecl & elt) override;
	  int32_t visit(const class FunDecl & elt) override;
	  int32_t visit(const class FunCall & elt) override;
	  int32_t visit(const class WhileLoop & elt) override;
	  int32_t visit(const class ForLoop & elt) override;
	  int32_t visit(const class Break & elt) override;
	  int32_t visit(const class Assign & elt) override;

};
}
#endif
