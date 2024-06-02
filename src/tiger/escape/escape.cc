#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TODO: Put your lab5 code here */
  root_->Traverse(env,0);
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto entry=env->Look(sym_);
  //esc::EscapeEntry *entry
  if(entry){
    if(entry->depth_<depth){
      *(entry->escape_)=true;
    }
  }

}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env,depth);
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env,depth);
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  for(auto x:args_->GetList()){
    x->Traverse(env,depth);
  }
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  left_->Traverse(env,depth);
  right_->Traverse(env,depth);

}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  for(auto x:fields_->GetList()){
    x->exp_->Traverse(env,depth);
  }
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */

  for(auto x:seq_->GetList()){
    x->Traverse(env,depth);
  }
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env,depth);
  exp_->Traverse(env,depth);

}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env,depth);
  then_->Traverse(env,depth);
  if(elsee_)elsee_->Traverse(env,depth);
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //*test_, *body_;
  test_->Traverse(env,depth);
  body_->Traverse(env,depth);
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //*var_,*lo_, *hi_, *body_;
  escape_=false;
  env->Enter(var_, new esc::EscapeEntry(depth,&escape_));
  lo_->Traverse(env,depth);
  hi_->Traverse(env,depth);
  body_->Traverse(env,depth);
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //decs_
  for(auto x:decs_->GetList()){
    x->Traverse(env,depth);
  }
  body_->Traverse(env,depth);
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  size_->Traverse(env,depth);
  init_->Traverse(env,depth);
  
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //functions_
  depth++;
  for(auto x:functions_->GetList()){
    for(auto y:x->params_->GetList()){
      //y->->Traverse(env,depth);
      y->escape_=false;
      env->Enter(y->name_, new esc::EscapeEntry(depth,&y->escape_));
    }
    x->body_->Traverse(env,depth);
  }


}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  init_->Traverse(env,depth);
  escape_=false;
  env->Enter(var_, new esc::EscapeEntry(depth,&escape_));
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

} // namespace absyn
