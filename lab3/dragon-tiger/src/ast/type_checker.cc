#include "type_checker.hh"

namespace ast {
namespace type_checker {

void TypeChecker::visit(IntegerLiteral &literal) {
  literal.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &literal) {
  literal.set_type(t_string);
}

void TypeChecker::visit(Sequence &seq) {
  std::vector<Expr *> exps = seq.get_exprs();
  //accept exprs
  for(size_t i = 0; i < exps.size(); i++) {
    exps[i]->accept(*this);
  }
  if(exps.size() == 0) {
    seq.set_type(t_void);
  }
  else {
    seq.set_type(exps.back()->get_type());
  } 
}

void TypeChecker::visit(IfThenElse &ite) {
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);
  
  if(ite.get_condition().get_type() != t_int) {
    utils::error(ite.loc, "Condition is not of type int");
  } 

  if(ite.get_then_part().get_type() != ite.get_else_part().get_type()) {
    utils::error(ite.loc, "Then and Else parts have different types");
  }
  else {
    ite.set_type(ite.get_then_part().get_type());
  }
}

void TypeChecker::visit(Let &let) {
  //accept decls
  for(size_t i = 0; i < let.get_decls().size(); i++) {
    let.get_decls()[i]->accept(*this);
  }
  let.get_sequence().accept(*this);
  let.set_type(let.get_sequence().get_type());
}

void TypeChecker::visit(VarDecl &decl) {
  if(decl.get_expr()) {
    decl.get_expr()->accept(*this);
  }
  if(!decl.type_name) {
    if(!decl.get_expr()) {
      utils::error(decl.loc, "Undeclared variable type");
    }
    else if(decl.get_expr()->get_type() == t_int || decl.get_expr()->get_type() == t_string) {
      decl.set_type(decl.get_expr()->get_type());
    }
    else {
      utils::error(decl.loc, "Unknown variable type");
    }
  }
  else {
    if(!decl.get_expr()) {
      if(decl.type_name.value().get() == "int") {
        decl.set_type(t_int);
      }
      else if(decl.type_name.value().get() == "string") {
        decl.set_type(t_string);
      }
      else {
        utils::error(decl.loc, "Unknown variable type");
      }
    }
    else {
      if(decl.type_name.value().get() == "int" 
		      && decl.get_expr()->get_type() == t_int) {
        decl.set_type(t_int);
      }
      else if(decl.type_name.value().get() == "string" 
		      && decl.get_expr()->get_type() == t_string) {
        decl.set_type(t_string);
      }
      else {
        utils::error(decl.loc, "Unknown variable type");
      }
    }
  }
}

void TypeChecker::visit(BinaryOperator &binOp) {
  binOp.get_left().accept(*this);
  binOp.get_right().accept(*this);
  if(binOp.get_left().get_type() != binOp.get_right().get_type()){
    utils::error(binOp.loc, "Different types in binary operation");
  }
  else if(binOp.get_left().get_type() == t_void) {
    utils::error(binOp.loc, "Cannot compare void in binary operation");
  }
  else if(binOp.op == o_eq || binOp.op == o_neq || binOp.op == o_lt 
		|| binOp.op == o_le || binOp.op == o_gt || binOp.op == o_ge){
    binOp.set_type(t_int);
  }
  else if(binOp.get_left().get_type() == t_int) {
    binOp.set_type(t_int);
  }
  else{
    utils::error(binOp.loc, "Type mismatch on binary operation"); 
  }
}

void TypeChecker::visit(Identifier &id) {
  id.set_type(id.get_decl()->get_type());
}

void TypeChecker::visit(Assign &assign) {
  assign.get_lhs().accept(*this);
  assign.get_rhs().accept(*this);

  if(assign.get_lhs().get_type() != assign.get_rhs().get_type()) {
    utils::error(assign.loc, "Type mismatch in assignement");
  }
  else {
    assign.set_type(t_void);
  }
}

void TypeChecker::visit(WhileLoop &loop) {
  loop.get_condition().accept(*this);
  loop.get_body().accept(*this);
  if(loop.get_condition().get_type() != t_int) {
    utils::error(loop.loc, "Condition should be of type int");
  }
  else if(loop.get_body().get_type() != t_void) {
    utils::error(loop.loc, "Body should be of type void");
  }
  else {
    loop.set_type(t_void);
  }
}

void TypeChecker::visit(ForLoop &loop) {
  loop.get_variable().accept(*this);
  loop.get_high().accept(*this);
  loop.get_body().accept(*this);

  if(loop.get_high().get_type() != t_int) {
    utils::error(loop.loc, "Type mismatch in loop high argument");
  }
  else if(loop.get_variable().get_type() != loop.get_high().get_type()) {
    utils::error(loop.loc, "Type mismatch in loop variable");
  }
  else if(loop.get_body().get_type() != t_void) {
    utils::error(loop.loc, "Type mismatch in loop body");
  }
  else {
    loop.set_type(t_void);
  }
}

void TypeChecker::visit(Break &b) {
  b.set_type(t_void);
}

void TypeChecker::visit(FunDecl &decl) {
  if(decl.get_type() != t_undef) {
    //declaration already visited
    return;
  }
  for(size_t i = 0; i < decl.get_params().size(); i++) {
    decl.get_params()[i]->accept(*this);  
  }
  
  if(!decl.type_name) {
    decl.set_type(t_void);
  }
  else{
    if(decl.type_name.value().get() == "int") {
      decl.set_type(t_int);
    }
    else if(decl.type_name.value().get() == "string") {
      decl.set_type(t_string);
    }
    else {
      utils::error(decl.loc, "Type mismatch in function declaration");
    }
  }

  if(decl.is_external) {
    if(decl.get_expr()) {
      decl.get_expr()->accept(*this);
    }
    return;
  }

  if(!decl.get_expr()) {
    if(decl.get_type() != t_void) {
      utils::error(decl.loc, "Type mismatch in function declaration");
    }
    else {
      return;
    }
  }

  else {
    decl.get_expr()->accept(*this);
  }


  if(decl.get_type() != decl.get_expr()->get_type()) {
    utils::error(decl.loc, "Type mismatch in function declaration");
  } 
}

void TypeChecker::visit(FunCall &call) {
  for(size_t i = 0; i < call.get_args().size(); i++) {
    call.get_args()[i]->accept(*this);
  }

  if(call.get_decl()->get_type() == t_undef) {
    call.get_decl()->accept(*this);
  }
  std::vector<Expr *> my_args = call.get_args();
  std::vector<VarDecl *> my_params = call.get_decl()->get_params();
  
  if(my_args.size() != my_params.size()) {
    utils::error(call.loc, "Incorrect number of arguments");
  }
  for(size_t i = 0; i < my_params.size(); i++) {
    if(my_params[i]->get_type() != my_args[i]->get_type()) {
      utils::error(call.loc, "Arguments type mismatch");
    }
  }
  call.set_type(call.get_decl()->get_type());
}

} // namespace type_checker
} // namespace ast
