#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TODO: Put your lab5 code here */
  if (!root_ || !env)
    throw std::invalid_argument("NULL pointer argument");
  root_->Traverse(env, 1);
  /* End for lab5 code */
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  esc::EscapeEntry *entry = env->Look(sym_);
  assert(entry != nullptr);
  if (depth > entry->depth_) {
    *(entry->escape_) = true;
  }
  /* End for lab5 code */
}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  /* End for lab5 code */
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  subscript_->Traverse(env, depth);
  /* End for lab5 code */
}

void VarExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  /* End for lab5 code */
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  if (!args_)
    return;
  for (Exp *arg : args_->GetList())
    arg->Traverse(env, depth);
  /* End for lab5 code */
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  left_->Traverse(env, depth);
  right_->Traverse(env, depth);
  /* End for lab5 code */
}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  for (EField *efield : fields_->GetList())
    efield->exp_->Traverse(env, depth);
  /* End for lab5 code */
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  for (Exp *seq_exp : seq_->GetList())
    seq_exp->Traverse(env, depth);
  /* End for lab5 code */
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  exp_->Traverse(env, depth);
  /* End for lab5 code */
}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);
  then_->Traverse(env, depth);
  if (elsee_)
    elsee_->Traverse(env, depth);
  /* End for lab5 code */
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);
  assert(body_);
  body_->Traverse(env, depth);
  /* End for lab5 code */
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  lo_->Traverse(env, depth);
  hi_->Traverse(env, depth);

  env->BeginScope();
  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &escape_));
  assert(body_);
  body_->Traverse(env, depth);
  env->EndScope();
  /* End for lab5 code */
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  env->BeginScope();
  for (Dec *dec : decs_->GetList())
    dec->Traverse(env, depth);

  assert(body_ != nullptr);
  body_->Traverse(env, depth);
  env->EndScope();
  /* End for lab5 code */
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  size_->Traverse(env, depth);
  init_->Traverse(env, depth);
  /* End for lab5 code */
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  for (FunDec *func_dec : functions_->GetList()) {
    env->BeginScope();
    if (func_dec->params_) {
      for (absyn::Field *param : func_dec->params_->GetList()) {
        param->escape_ = false;
        env->Enter(param->name_,
                   new esc::EscapeEntry(depth + 1, &param->escape_));
      }
    }
    func_dec->body_->Traverse(env, depth + 1);
    env->EndScope();
  }
  /* End for lab5 code */
}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &escape_));
  init_->Traverse(env, depth);
  /* End for lab5 code */
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  /* End for lab5 code */
}

} // namespace absyn
