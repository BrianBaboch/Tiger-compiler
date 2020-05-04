#include "evaluator.hh"

namespace ast{

int32_t Evaluator::visit(const class IntegerLiteral & elt){
	return(elt.value);
}
int32_t Evaluator::visit(const class StringLiteral & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class BinaryOperator & elt){
	int32_t left = elt.get_left().accept(*this);
	int32_t right = elt.get_right().accept(*this);
	Operator oper = elt.op;

	if(oper == o_plus){return (left + right);}
	if(oper == o_minus){return (left - right);}
	if(oper == o_times){return (left * right);}
	if(oper == o_divide){return (left / right);}
  	if(oper == o_eq){
		if(right == left){return 1;}
		else {return 0;}
	}
	if(oper == o_eq){
		if(left == right){return 1;}
		else {return 0;}
	}
	if(oper == o_neq){
		if(left != right){return 1;}
		else {return 0;}
	}
	if(oper == o_lt){
		if(left < right){return 1;}
		else {return 0;}
	}
	if(oper == o_le){
		if(left <= right){return 1;}
		else {return 0;}
	}
	if(oper == o_gt){
		if(left > right){return 1;}
		else {return 0;}
	}
	if(oper == o_ge){
		if(left >= right){return 1;}
		else {return 0;}
	}
	utils::error("Operator not recognized");
}
int32_t Evaluator::visit(const class Sequence & elt){
	std::vector<Expr *> exp = elt.get_exprs();
	if(exp.size() == 0){utils::error("The Sequence is empty");}
	else{
		int32_t eval = 0;
		for(size_t i = 0; i < exp.size(); ++i){
			eval = exp[i]->accept(*this);
		}
		return(eval);
	}
}
int32_t Evaluator::visit(const class Let & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class Identifier & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class IfThenElse & elt){
	if(elt.get_condition().accept(*this)){return elt.get_then_part().accept(*this);}
	else {return elt.get_else_part().accept(*this);}
}
int32_t Evaluator::visit(const class VarDecl & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class FunDecl & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class FunCall & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class WhileLoop & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class ForLoop & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class Break & elt){
	utils::error("Visitor not defined");
}
int32_t Evaluator::visit(const class Assign & elt){
	utils::error("Visitor not defined");
}
}
