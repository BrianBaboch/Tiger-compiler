#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  const Symbol s_int = Symbol("int");
  const Symbol s_string = Symbol("string");
  enter_primitive("print_err", boost::none, {s_string});
  enter_primitive("print", boost::none, {s_string});
  enter_primitive("print_int", boost::none, {s_int});
  enter_primitive("flush", boost::none, {});
  enter_primitive("getchar", s_string, {});
  enter_primitive("ord", s_int, {s_string});
  enter_primitive("chr", s_string, {s_int});
  enter_primitive("size", s_int, {s_string});
  enter_primitive("substring", s_string, {s_string, s_int, s_int});
  enter_primitive("concat", s_string, {s_string, s_string});
  enter_primitive("strcmp", s_int, {s_string, s_string});
  enter_primitive("streq", s_int, {s_string, s_string});
  enter_primitive("not", s_int, {s_int});
  enter_primitive("exit", boost::none, {s_int});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const boost::optional<Symbol> &type_name,
    const std::vector<Symbol> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const Symbol &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), nullptr, tn));
  }

  boost::optional<Symbol> type_name_symbol = boost::none;
  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), std::move(args), nullptr,
                            type_name, true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), main_params,
                                    main_body, Symbol("int"), true);
  main->accept(*this);
  return main;
}

void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op) {
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq) {
  std::vector<Expr *> exp = seq.get_exprs();
  for(size_t i = 0; i < exp.size(); i++){
    exp[i]->accept(*this);
  }
}

void Binder::visit(Let &let) {
  push_scope();
  std::vector<Decl *> dec = let.get_decls();
  std::vector<FunDecl *> func;
  for(size_t i = 0; i < dec.size(); i++){
    //Function Declaration
    if(dynamic_cast<FunDecl *>(dec[i])){
      let_bloc = false;
      enter(* dec[i]);
      func.push_back(dynamic_cast<FunDecl *>(dec[i]));
    }
    //Variable Declaration
    else{
      //Accept all consecutive functions
      while(func.size() > 0){
        func[0]->accept(*this);
	func.erase(func.begin());
      }
      let_bloc = true;
      //Accept Variable Declaration
      dec[i]->accept(*this);
    }
  }
  let_bloc = false;
  //Accept all consecutive functions if there were no VarDecls
  if(func.size() > 0){
    while(func.size() > 0){
      func[0]->accept(*this);
      func.erase(func.begin());
    }
  }
  let.get_sequence().accept(*this);
  pop_scope();
}

void Binder::visit(Identifier &id) {
  VarDecl * tmp = dynamic_cast<VarDecl *>(&find(id.loc, id.name));
  if(tmp) {
    id.set_decl(tmp);
    id.set_depth(current_depth);
    if(tmp->get_depth() != current_depth){
      tmp->set_escapes();
    }
  }
  else {
    utils::error(id.loc, "Declaration is not a variable");
  }
}

void Binder::visit(IfThenElse &ite) {
  (ite.get_condition()).accept(*this);
  (ite.get_then_part()).accept(*this);
  (ite.get_else_part()).accept(*this);
}

void Binder::visit(VarDecl &decl) {
  decl.set_depth(current_depth);
  if(decl.get_expr()){
    decl.get_expr()->accept(*this);
  }
  enter(decl);
}

void Binder::visit(FunDecl &decl) {
  set_parent_and_external_name(decl);
  functions.push_back(&decl);

  push_scope();
  decl.set_depth(current_depth);
  current_depth = current_depth + 1;
  //Accept all parameters
  std::vector<VarDecl *> parameters = decl.get_params();
  for(size_t i = 0; i < parameters.size(); i++){
    parameters[i]->accept(*this);
  }
  if(decl.get_expr()){
    decl.get_expr()->accept(*this);
  }
  else{
    utils::error("Function not defined");
  }
  current_depth = current_depth - 1;
  //Accept escaping declarations
  std::vector<VarDecl *> escaping_decls = decl.get_escaping_decls();
  for(size_t i = 0; i < escaping_decls.size(); i++){
    escaping_decls[i]->accept(*this);
  } 
  pop_scope();
  functions.pop_back();
}

void Binder::visit(FunCall &call) {
  call.set_depth(current_depth);
  if(dynamic_cast<FunDecl *>(&find(call.loc, call.func_name))){
    call.set_decl(dynamic_cast<FunDecl *>(&find(call.loc, call.func_name)));
    std::vector<Expr *> exp = call.get_args();
    for(size_t i = 0; i < exp.size(); i++){
      exp[i]->accept(*this);
    }
  }
  else {
    utils::error(call.loc, "Declaration is not a function");
  } 
}

void Binder::visit(WhileLoop &loop) {
  push_scope();
  current_depth = current_depth + 1;
  loop.get_condition().accept(*this);
  visited_loops.push_back(&loop);
  loop.get_body().accept(*this);
  current_depth = current_depth - 1;
  pop_scope();
  visited_loops.pop_back();
}

void Binder::visit(ForLoop &loop) {
  loop.get_high().accept(*this);
  push_scope();
  current_depth = current_depth + 1;
  loop.get_variable().accept(*this);
  visited_loops.push_back(&loop);
  loop.get_body().accept(*this);
  current_depth = current_depth - 1;
  pop_scope();
  visited_loops.pop_back();
}

void Binder::visit(Break &b) { 
  if(visited_loops.size() == 0) {
    utils::error("There are no loops");
  }
  else {
    if(let_bloc) {
      utils::error("Cannot have a break inside VarDecl");
    }
    else {
      b.set_loop(visited_loops.back());
    }
  }
}

void Binder::visit(Assign &assign) {
  (assign.get_rhs()).accept(*this);
  (assign.get_lhs()).accept(*this);
  if(((assign.get_lhs()).get_decl())->read_only){
    utils::error("This variable is not assignable");
  }
}

} // namespace binder
} // namespace ast
