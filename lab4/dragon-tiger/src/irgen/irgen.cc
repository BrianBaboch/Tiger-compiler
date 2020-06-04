#include "irgen.hh"
#include "../utils/errors.hh"

#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using utils::error;

namespace irgen {

IRGenerator::IRGenerator() : Builder(Context) {
  Mod = llvm::make_unique<llvm::Module>("tiger", Context);
}

llvm::Type *IRGenerator::llvm_type(const ast::Type ast_type) {
  switch (ast_type) {
  case t_int:
    return Builder.getInt32Ty();
  case t_string:
    return Builder.getInt8PtrTy();
  case t_void:
    return Builder.getVoidTy();
  default:
    assert(false); __builtin_unreachable();
  }
}

llvm::Value *IRGenerator::alloca_in_entry(llvm::Type *Ty,
                                          const std::string &name) {
  llvm::IRBuilderBase::InsertPoint const saved = Builder.saveIP();
  Builder.SetInsertPoint(&current_function->getEntryBlock());
  llvm::Value *const value = Builder.CreateAlloca(Ty, nullptr, name);
  Builder.restoreIP(saved);
  return value;
}

void IRGenerator::print_ir(std::ostream *ostream) {
  // FIXME: This is inefficient. Should probably take a filename
  // and use directly LLVM raw stream interface
  std::string buffer;
  llvm::raw_string_ostream OS(buffer);
  OS << *Mod;
  OS.flush();
  *ostream << buffer;
}

llvm::Value *IRGenerator::address_of(const Identifier &id) {
  assert(id.get_decl());
  const VarDecl &decl = dynamic_cast<const VarDecl &>(id.get_decl().get());
  if(decl.get_escapes()) {
    std::pair<llvm::StructType *, llvm::Value *> myFrame = 
	    frame_up(id.get_depth() - decl.get_depth());
    return (Builder.CreateStructGEP(myFrame.first, myFrame.second, 
			    frame_position[&decl]));
  }
  else {
    return allocations[&decl];
  }
}

void IRGenerator::generate_program(FunDecl *main) {
  main->accept(*this);

  while (!pending_func_bodies.empty()) {
    generate_function(*pending_func_bodies.back());
    pending_func_bodies.pop_back();
  }
}

void IRGenerator::generate_function(const FunDecl &decl) {
  // Reinitialize common structures.
  allocations.clear();
  loop_exit_bbs.clear();

  // Set current function
  current_function = Mod->getFunction(decl.get_external_name().get());
  current_function_decl = &decl;
  std::vector<VarDecl *> params = decl.get_params();

  // Create a new basic block to insert allocation insertion
  llvm::BasicBlock *bb1 =
      llvm::BasicBlock::Create(Context, "entry", current_function);

  // Create a second basic block for body insertion
  llvm::BasicBlock *bb2 =
      llvm::BasicBlock::Create(Context, "body", current_function);

  Builder.SetInsertPoint(bb2);

  generate_frame();


  // Set the name for each argument and register it in the allocations map
  // after storing it in an alloca.
  unsigned i = 0;
  if(!decl.is_external) {
    llvm::Value * struc = Builder.CreateStructGEP(frame_type[&decl], frame, 0);
    Builder.CreateStore(allocations[params[0]], struc);
  }
  for (auto &arg : current_function->args()) {
    arg.setName(params[i]->name.get());
    //llvm::Value *const shadow = alloca_in_entry(llvm_type(params[i]->get_type()), params[i]->name.get());


    llvm::Value *const shadow = generate_vardecl(*params[i]);
    Builder.CreateStore(&arg, shadow);
    i++;
  }

  // Visit the body
  llvm::Value *expr = decl.get_expr()->accept(*this);

  // Finish off the function.
  if (decl.get_type() == t_void)
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(expr);

  // Jump from entry to body
  Builder.SetInsertPoint(bb1);
  Builder.CreateBr(bb2);

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*current_function);
}

void IRGenerator::generate_frame() {
  std::vector<llvm::Type *> types;
  if(current_function_decl->get_parent()) {
    types.push_back(frame_type[
	&current_function_decl->get_parent().get()]->getPointerTo());
    for (auto escp_decl : current_function_decl->get_escaping_decls()) {
      if(escp_decl->get_type() != t_void) {
        types.push_back(llvm_type(escp_decl->get_type()));
      }
    }
  }
  llvm::StructType * myStruct = llvm::StructType::create(Context, "ft_" + 
	  current_function_decl->get_external_name().get());
  myStruct->setBody(types);
  frame_type[current_function_decl] = myStruct;

  frame = Builder.CreateAlloca(myStruct, nullptr, 
		  "ft" + current_function_decl->get_external_name().get());
}

std::pair<llvm::StructType *, llvm::Value *> IRGenerator::frame_up(int levels) {
  const FunDecl * fun = current_function_decl;
  llvm::Value * sl = frame;
  int current_level = levels;
  while(current_level > 0) {
    sl = Builder.CreateLoad(Builder.CreateStructGEP(frame_type[fun], sl, 0)); 
    fun = &fun->get_parent().get();
    current_level = current_level - 1;
  }
  std::pair<llvm::StructType *, llvm::Value *> myPair;
  myPair.first = frame_type[fun];
  myPair.second = sl;
  return myPair;
}

llvm::Value *IRGenerator::generate_vardecl(const VarDecl &decl) {
  if(decl.get_escapes()) {
    int index = 0;
    for(size_t i = 0; i < (current_function_decl->get_escaping_decls()).size(); 
		    i++) {

      if(current_function_decl->get_escaping_decls()[i]->get_type() != t_void) {
        index += 1;
      }

      if(current_function_decl->get_escaping_decls()[i] == &decl) {
	break;
      }

    }
    frame_position[&decl] = index;
    llvm::Value * adr = Builder.CreateStructGEP(
		    frame_type[current_function_decl], frame, index);
    allocations[&decl] = adr;
    return adr;
  }
  else {
    llvm::Type * varType = llvm_type(decl.get_type());
    llvm::Value * varPtr = alloca_in_entry(varType, decl.name);
    allocations[&decl] = varPtr;
    return varPtr;
  }
}
} // namespace irgen
