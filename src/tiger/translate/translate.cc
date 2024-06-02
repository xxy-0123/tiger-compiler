#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;

namespace {
frame::ProcFrag *ProcEntryExit(tr::Level *level, tr::Exp *body);
}

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  return new Access(level, level->frame_->AllocLocal(escape));
}

class Cx {
public:
  temp::Label **trues_;
  temp::Label **falses_;
  tree::Stm *stm_;

  Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : trues_(trues), falses_(falses), stm_(stm) {}
};

class Exp {
public:
  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    return exp_;

  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(exp_);

  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */

  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::EseqExp(stm_, new tree::ConstExp(0));

  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    return stm_;

  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    return Cx(nullptr,nullptr,nullptr);
  }
};

class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
      : cx_(trues, falses, stm) {}
  
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    temp::Temp *r = temp::TempFactory::NewTemp();
    temp::Label *t = temp::LabelFactory::NewLabel();
    temp::Label *f = temp::LabelFactory::NewLabel();
    *cx_.trues_=t; 
    *cx_.falses_=f;
    return new tree::EseqExp(
      new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(1)),
      new tree::EseqExp(
        cx_.stm_,
        new tree::EseqExp(
          new tree::LabelStm(f),
          new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r),
            new tree::ConstExp(0)),
            new tree::EseqExp(new tree::LabelStm(t),
            new tree::TempExp(r))))));
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::SeqStm(cx_.stm_,nullptr);
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  // absyn_tree_(std::move(absyn_tree)), errormsg_(std::move(errormsg)),
  //       main_level_(std::make_unique<Level>(
  //           frame::NewFrame(temp::LabelFactory::NamedLabel("tigermain"),
  //                           std::list<bool>()),
  //           nullptr)),
  //       tenv_(std::make_unique<env::TEnv>()),
  //       venv_(std::make_unique<env::VEnv>()) {}
  FillBaseTEnv();
  FillBaseVEnv();
  //temp::Label *label=temp::LabelFactory::NamedLabel("tigermain");

}

} // namespace tr

namespace {

/**
 * Wrapper for `ProcExitEntry1`, which deals with the return value of the
 * function body
 * @param level current level
 * @param body function body
 * @return statements after `ProcExitEntry1`
 */
frame::ProcFrag *ProcEntryExit(tr::Level *level, tr::Exp *body) {
  /* TODO: Put your lab5 code here */
  frame::ProcEntryExit1(level->frame_, body->UnNx());
  return body->UnNx();
}
} // namespace

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return root_->Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto ventry = (env::VarEntry*)venv->Look(sym_);
  tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
  while(ventry->access_->level_!=level){
    exp=level->frame_->formals_->front()->ToExp(exp);
    level=level->parent_;
  }
   return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *var = var_->Translate(venv, tenv, level, label, errormsg);
  auto ventry = (env::VarEntry*)venv->Look(sym_);
  tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
  while(ventry->access_->level_!=level){
    exp=level->frame_->formals_->front()->ToExp(exp);
    level=level->parent_;
  }
  return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *var = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *subscript = subscript_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *exp_mem=new tr::ExExp(new tree::MemExp(new tree::BinopExp(tree::PLUS_OP,var->exp_->UnEx(),new tree::BinopExp(tree::MUL_OP,subscript->exp_->UnEx(),new tree::ConstExp(8)))));
  return new tr::ExpAndTy(exp_mem,var->ty_->ActualTy());
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(new tr::ExExp(nullptr),type::NilTy::Instance());
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(val_)),type::IntTy::Instance());
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto *lib=temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(lib, str_));
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(lib)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto l=left_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  auto r=right_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tree::CjumpStm *cj;
  switch(oper_){
    case PLUS_OP:
      return new tr::ExpAndTy(new tr::ExExp(new tree::BinopExp(tree::PLUS_OP,l,r)),type::IntTy::Instance());
    case MINUS_OP:
      return new tr::ExpAndTy(new tr::ExExp(new tree::BinopExp(tree::MINUS_OP,l,r)),type::IntTy::Instance());
    case DIVIDE_OP:
      return new tr::ExpAndTy(new tr::ExExp(new tree::BinopExp(tree::DIV_OP,l,r)),type::IntTy::Instance());
    case TIMES_OP:
      return new tr::ExpAndTy(new tr::ExExp(new tree::BinopExp(tree::MUL_OP,l,r)),type::IntTy::Instance());
    case LT_OP:
      cj=new tree::CjumpStm(tree::RelOp(tree::LT_OP),l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    case LE_OP:
      cj=new tree::CjumpStm(tree::RelOp(tree::LE_OP),l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    case GT_OP:
      cj=new tree::CjumpStm(tree::RelOp(tree::GT_OP),l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    case GE_OP:
      cj=new tree::CjumpStm(tree::RelOp(tree::GE_OP),l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    case EQ_OP:
      if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      }
      else{
        cj=new tree::CjumpStm(tree::RelOp(tree::EQ_OP),l,r,nullptr,nullptr);
        temp::Label** trues=&cj->true_label_;
        temp::Label** falses=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(trues,falses,cj);
        return new tr::ExpAndTy(exp, type::IntTy::Instance());
      }
    case NEQ_OP:
      if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      }
      else{
        cj=new tree::CjumpStm(tree::RelOp(tree::NE_OP),l,r,nullptr,nullptr);
        temp::Label** trues=&cj->true_label_;
        temp::Label** falses=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(trues,falses,cj);
        return new tr::ExpAndTy(exp, type::IntTy::Instance());
      }
  }

}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::Exp *exp=nullptr;
  type::Ty *ty;
  for(auto x:seq_->GetList()){
    if(!exp){
      exp=new tr::ExExp(x->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx());
      ty=x->Translate(venv,tenv,level,label,errormsg)->ty_->ActualTy();
    }
    else{
      exp=new tr::ExExp(new tree::EseqExp(exp->UnNx(),x->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx()));
      ty=x->Translate(venv,tenv,level,label,errormsg)->ty_->ActualTy();
    }
  }
  return new tr::ExpAndTy(exp, ty); 
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tree::Exp *ET_var = var_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tree::Exp *ET_exp = exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tr::Exp *exp=new tr::NxExp(new tree::MoveStm(ET_var,ET_exp));
  return new tr::ExpAndTy(exp, type::VoidTy::Instance()); 

}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
 
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::vector<temp::Label *> *jumps_;
  jumps_->push_back(label);
  tree::Stm *stm=new tree::JumpStm(new tree::NameExp(label), jumps_);
  return new tr::ExpAndTy(new tr::NxExp(stm), type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tree::SeqStm *stmlist=nullptr;
  tree::Stm *stm=nullptr;
  for(auto x:decs_->GetList()){
    if(!stm){
      stm=x->Translate(venv, tenv, level, label, errormsg)->UnNx();
    }
    else{
      if(!stmlist){
        tree::Stm *stm2=x->Translate(venv, tenv, level, label, errormsg)->UnNx();
        *stmlist=tree::SeqStm(stm,stm2);
      }
      else{
        stm=x->Translate(venv, tenv, level, label, errormsg)->UnNx();
        *stmlist=tree::SeqStm(stm,stmlist);
      }      
    }
  }
  tr::ExpAndTy *let_body=body_->Translate(venv, tenv, level, label, errormsg);
  tree::Exp *exp=let_body->exp_->UnEx();
  tree::Exp *rt_exp=new tree::EseqExp(stmlist,exp);

  return new tr::ExpAndTy(new tr::ExExp(rt_exp),let_body->ty_->ActualTy());

}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
//   tree::Exp *ExternalCall(std::string_view s, tree::ExpList *args) {
//   // Prepend a magic exp at first arg, indicating do not pass static link on
//   // stack
//   args->Insert(new tree::NameExp(temp::LabelFactory::NamedLabel("staticLink")));
//   return new tree::CallExp(new tree::NameExp(temp::LabelFactory::NamedLabel(s)),
//                            args);
// }
  tree::Exp *arr_size=size_->Translate(venv, tenv, level, label, errormsg)->exp_->UnEx();
  tree::Exp *arr_init=init_->Translate(venv, tenv, level, label, errormsg)->exp_->UnEx();
  tree::ExpList *args=new tree::ExpList();
  args->Append(arr_size);
  args->Append(arr_init);
  tr::Exp *exp = new tr::ExExp(frame::ExternalCall("initArray", args));
  return new tr::ExpAndTy(exp,tenv->Look(typ_)->ActualTy()); 
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(nullptr, type::VoidTy::Instance());
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
 return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //return new type::NameTy(name_,tenv->Look(name_));
  type::Ty *ty = tenv->Look(name_);
  return ty;
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto record_list = record_->MakeFieldList(tenv,errormsg);
  return new type::RecordTy(record_list);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto ty = tenv->Look(array_);
  return new type::ArrayTy(ty);
}

} // namespace absyn
