
#include "tiger/absyn/absyn.h"
#include "tiger/semant/semant.h"
#include <set>
#include <iterator> 

namespace absyn {

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  root_->SemAnalyze(venv, tenv, 0, errormsg);
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(sym_);
  if (entry && typeid(*entry) == typeid(env::VarEntry)) {
      return (static_cast<env::VarEntry *>(entry))->ty_->ActualTy();
  } else {
    errormsg->Error(pos_, "undefined variable %s", sym_->Name().data());
  }
  return type::IntTy::Instance();
}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(!ty||typeid(*ty)!=typeid(type::RecordTy)){
    errormsg->Error(pos_, "not a record type");
    return type::IntTy::Instance();
  }
  else {
    for(auto field:((type::RecordTy*)ty)->fields_->GetList()){
      if(field->name_==sym_){
        return field->ty_->ActualTy();
      }
    }
    errormsg->Error(pos_, "field %s doesn't exist", sym_->Name().data());
    return type::IntTy::Instance();
  }
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if(typeid(*ty)!=typeid(type::ArrayTy))
  {
    errormsg->Error(pos_, "array type required");
  }
  else {
    type::Ty *ty_sub = subscript_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if(typeid(*ty_sub)!=typeid(type::IntTy)){
      errormsg->Error(pos_, "IntTy error");
    }
    else return (static_cast<type::ArrayTy*>(ty))->ty_->ActualTy();
  }
  return type::IntTy::Instance();
}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  return ty;
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto ventry=(env::FunEntry*)(venv->Look(func_));
  if(!ventry||typeid(*ventry)!=typeid(env::FunEntry)){
    errormsg->Error(pos_, "undefined function %s", func_->Name().data());
    return type::IntTy::Instance();
  }//||typeid(*ventry)!=
  auto ty_flist=ventry->formals_->GetList();
  auto arg_list=args_->GetList();
  auto itr_formals=ty_flist.begin();
  auto itr_args=arg_list.begin();
  while(itr_formals!=ty_flist.end()&&itr_args!=arg_list.end()){
    auto ty_arg = (*itr_args)->SemAnalyze(venv, tenv, labelcount, errormsg);
    if(!(*itr_formals)->IsSameType(ty_arg)){
      //if(typeid(ty_arg)!=typeid(*itr_formals)) 
      // errormsg->Error(pos_, "para type mismatch");
      // return type::IntTy::Instance();
      break;
    }
    itr_formals++;
    itr_args++;
  }

// 计算链表长度
  auto len_f = std::distance(ty_flist.begin(), ty_flist.end());
  auto len_a = std::distance(arg_list.begin(), arg_list.end());
  if(len_f>len_a){
    errormsg->Error(pos_, "too few params in function %s", func_->Name().data());//params in function 
    //return type::IntTy::Instance();
  }
  else if(len_f<len_a){
    errormsg->Error(pos_, "too many params in function %s", func_->Name().data());
    //return type::IntTy::Instance();
  }
  if(itr_formals!=ty_flist.end()&&itr_args!=arg_list.end()){
      errormsg->Error(pos_, "para type mismatch");
      return type::IntTy::Instance();
  }

  return ventry->result_;
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  type::Ty *left_ty = left_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *right_ty= right_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if (oper_ == absyn::PLUS_OP || oper_ == absyn::MINUS_OP || oper_ == absyn::TIMES_OP || oper_ == absyn::DIVIDE_OP) {
      if (typeid(*left_ty) != typeid(type::IntTy)) {
        errormsg->Error(left_->pos_,"integer required");
      }
      if (typeid(*right_ty) != typeid(type::IntTy)) {
        errormsg->Error(right_->pos_,"integer required");
      }
      return type::IntTy::Instance();
    }
  if (!left_ty->IsSameType(right_ty)) {
      errormsg->Error(pos_, "same type required");
    }
  return type::IntTy::Instance();
  /* TODO: Put your lab4 code here */
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = tenv->Look(typ_);
  if(!ty) {
    errormsg->Error(pos_,"undefined type %s",typ_->Name().data());
    return type::IntTy::Instance();
  } 
  else{
    // if(typeid(*ty)!=typeid(type::RecordTy)){
    //   errormsg->Error(pos_, "not 1 a record type");
    //   return type::IntTy::Instance();
    // }
    ty=ty->ActualTy();
    auto t_list=((type::RecordTy*)ty)->fields_->GetList();
    auto f_list=fields_->GetList();
    for(auto field : f_list){
      field->exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
    }
  }
  return ty;
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto seq=seq_->GetList().front();
  type::Ty *ty ;
  if(!seq){
    return type::VoidTy::Instance();
  }
  for(auto exp_: seq_->GetList()){
    ty=exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
  }
  return ty;
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty_test = test_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_test)!=typeid(type::IntTy)){
    errormsg->Error(pos_, "test_ required");
    return type::IntTy::Instance();
  }
  if(elsee_){
    type::Ty *ty_th = then_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
    type::Ty *ty_el = elsee_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
      if(typeid(*ty_th)!=typeid(*ty_el)){
        errormsg->Error(pos_, "then exp and else exp type mismatch");
        return type::IntTy::Instance();
      }
      return ty_el;
  }
  else{
    type::Ty *ty_th = then_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
    if(typeid(*ty_th)!=typeid(type::VoidTy)){
      errormsg->Error(pos_, "if-then exp's body must produce no value");
      return type::VoidTy::Instance();
    }
    else return ty_th;
  }
  return type::IntTy::Instance();
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty_test = test_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_test)!=typeid(type::IntTy)){
    errormsg->Error(pos_, "test_ required");
    return type::IntTy::Instance();
  }
  type::Ty *ty_bd = body_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_bd)!=typeid(type::VoidTy)){
    errormsg->Error(pos_, "while body must produce no value");
    return type::VoidTy::Instance();
  }
  else return ty_bd;
  return type::VoidTy::Instance();
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty_var = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *ty_exp = exp_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*var_) == typeid(SimpleVar)) {
    auto simple_var = (SimpleVar*)var_;
    auto entry = venv->Look(simple_var->sym_);
    if(entry->readonly_) {
      errormsg->Error(pos_, "loop variable can't be assigned");
    }
  }
  if(!ty_exp->IsSameType(ty_var)) {
    errormsg->Error(pos_, "unmatched assign exp");
    return type::VoidTy::Instance();
  }
  return type::VoidTy::Instance(); 
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto tmp = venv->Look(var_);
  if(tmp){
    if(tmp->readonly_){
      errormsg->Error(pos_,"loop variable can't be assigned");
      return type::VoidTy::Instance();
    }
  }
  type::Ty *ty_lo = lo_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_lo)!=typeid(type::IntTy)){
    errormsg->Error(lo_->pos_, "for exp's range type is not integer");
  }
  type::Ty *ty_hi = hi_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_hi)!=typeid(type::IntTy)){
    errormsg->Error(hi_->pos_, "for exp's range type is not integer");
  }
  venv->BeginScope();
  venv->Enter(var_, new env::VarEntry(type::IntTy::Instance(), true));
  body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);
  venv->EndScope();
  return type::VoidTy::Instance(); 
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if(labelcount==0)
    errormsg->Error(pos_, "loop variable can't be assigned");
  return type::IntTy::Instance(); 
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  venv->BeginScope();
  tenv->BeginScope();
  for (Dec *dec : decs_->GetList())
    dec->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *result;
  if (!body_) 
    {result = type::VoidTy::Instance();}
  else 
    result = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  tenv->EndScope();
  venv->EndScope();
  return result;                            
  /* TODO: Put your lab4 code here */
}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = tenv->Look(typ_)->ActualTy();
  // if(!ty){
  //   errormsg->Error(pos_, "typ_ 111 error");
  //   return type::IntTy::Instance();
  // }
  // else if(typeid(*ty)!=typeid(type::ArrayTy)){
  //   errormsg->Error(pos_, "ArrayTy 111 required");
  //   return type::IntTy::Instance();
  // }
  // type::Ty *ty_s = tenv->Look(typ_)->ActualTy();
  // if(typeid(*ty_s)!=typeid(type::IntTy)){
  //   errormsg->Error(pos_, "IntTy error");
  //   return type::IntTy::Instance();
  // }
  type::Ty *ty_s = size_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *ty_init = init_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if(!ty_init->IsSameType(((type::ArrayTy*)ty)->ty_)){
    errormsg->Error(pos_, "type mismatch");
    return type::IntTy::Instance();
  }
  return ty;
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  std::set<sym::Symbol*> symble_table; 
  for(auto function :functions_->GetList()){
    if(symble_table.count(function->name_)){
      errormsg->Error(pos_, "two functions have the same name");
      return;
    }
    else{
      symble_table.insert(function->name_);
    }
    absyn::FieldList *params = function->params_;
    auto formals = params->MakeFormalTyList(tenv, errormsg);
    type::Ty* result_ty;
    if(function->result_) 
      result_ty = tenv->Look(function->result_);
    else {
      result_ty = type::VoidTy::Instance();
    }
    venv->Enter(function->name_, new env::FunEntry(formals, result_ty));
  }
  for(auto function :functions_->GetList()){
    absyn::FieldList *params = function->params_;
    auto formals = params->MakeFormalTyList(tenv, errormsg);
    venv->BeginScope();
    auto formal_it = formals->GetList().begin();
    auto param_it = params->GetList().begin();
    for (; param_it != params->GetList().end(); formal_it++, param_it++)
      venv->Enter((*param_it)->name_, new env::VarEntry(*formal_it));
    type::Ty *ty;
    ty= function->body_->SemAnalyze(venv, tenv, labelcount, errormsg);
    type::Ty *ty_ret;
    if(function->result_)
      ty_ret = tenv->Look(function->result_)->ActualTy();
    else ty_ret= type::VoidTy::Instance();
    if(!ty->IsSameType(ty_ret)) {
      //if(typeid(*ty_ret) == typeid(type::VoidTy)) {
        errormsg->Error(pos_, "procedure returns value");
      //}
      // } else {
      //   errormsg->Error(pos_, "return value mismatch");
      // }
    }
    venv->EndScope();
  }
}


void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *init_ty = init_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if(!typ_){
    if(typeid(*init_ty) == typeid(type::NilTy)) {
      errormsg->Error(pos_, "init should not be nil without type specified");
    }
  }else{
    if(!init_ty->IsSameType(tenv->Look(typ_))){
    errormsg->Error(pos_, "type mismatch");
    }
  }
  venv->Enter(var_, new env::VarEntry(init_ty));
}

void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  std::set<sym::Symbol*> symble_table; 
  for (auto itr_type : types_->GetList()) {
    if(symble_table.count(itr_type->name_)){
      errormsg->Error(pos_, "two types have the same name");
    }
    else{
      symble_table.insert(itr_type->name_);
      tenv->Enter(itr_type->name_, new type::NameTy(itr_type->name_, nullptr));
    }
  }
  for (auto itr_type : types_->GetList()) {
    auto *ty_name=(type::NameTy*)tenv->Look(itr_type->name_);
    ty_name->ty_=itr_type->ty_->SemAnalyze(tenv, errormsg);
  }
  //above is for recursive declare
  auto ty_list = types_->GetList();
  for(auto type : ty_list) {
    auto *ty_cur = tenv->Look(type->name_);
    auto *ty_itr = ty_cur;
    while(typeid(*ty_itr)==typeid(type::NameTy)) {
      ty_itr = ((type::NameTy*)ty_itr)->ty_;
      if(((type::NameTy*)ty_itr)->sym_==((type::NameTy*)ty_cur)->sym_) {
        errormsg->Error(pos_, "illegal type cycle");
        return;
      }
    }
  }

}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = tenv->Look(name_);
  return ty;
}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto record_list = record_->MakeFieldList(tenv,errormsg);
  return new type::RecordTy(record_list);
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto ty = tenv->Look(array_);
  return new type::ArrayTy(ty);
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}

} // namespace sem当初错怪你了