```cpp
#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

#include <iostream>

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
    tree::CjumpStm *cjstm=new tree::CjumpStm(tree::NE_OP,exp_,new tree::ConstExp(0),nullptr,nullptr);
    cjstm->true_label_=temp::LabelFactory::NewLabel();
    cjstm->true_label_=temp::LabelFactory::NewLabel();
    return tr::Cx(&(cjstm->true_label_), &(cjstm->false_label_), cjstm);
    
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
    return cx_.stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */

  FillBaseVEnv();
  FillBaseTEnv();
  temp::Label *main_label_ = temp::LabelFactory::NamedLabel("tigermain");
  tr::ExpAndTy *exp_ty = absyn_tree_->Translate(venv_.get(), tenv_.get(), main_level_.get(), main_label_, errormsg_.get());
  frags->PushBack(new frame::ProcFrag(frame::ProcEntryExit1(main_level_->frame_,exp_ty->exp_->UnNx()), main_level_->frame_));


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
  printf("\n begin procEntryExit\n");
  // frame::ProcEntryExit1(level->frame_, body->UnNx());
  // return body->UnNx();
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
  std::cout<<"---SimpleVar---"<<std::endl;
  auto *entry = venv->Look(sym_);
  auto *ventry = (env::VarEntry*)venv->Look(sym_);
  if(entry && typeid(*entry)==typeid(env::VarEntry))
  {tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
  while(ventry->access_->level_!=level){
    exp=level->frame_->formals_.front()->ToExp(exp);
    level=level->parent_;
  }

  return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());}
  else{
    return new tr::ExpAndTy(nullptr, type::IntTy::Instance());
  }
  // tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
  // while(ventry->access_->level_!=level){
  //   exp=level->frame_->formals_.front()->ToExp(exp);
  //   level=level->parent_;
  // }

  // return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---FieldVar---"<<std::endl;
  tr::ExpAndTy *var = var_->Translate(venv, tenv, level, label, errormsg);
  int cnt=0;
  for(auto *x:((type::RecordTy*)var->ty_)->fields_->GetList()){
    if(x->name_==sym_){
      tr::Exp *exp = new tr::ExExp(new tree::MemExp(
        new tree::BinopExp(tree::PLUS_OP,new tree::ConstExp(cnt*8),var->exp_->UnEx())
      ));
      return new tr::ExpAndTy(exp, x->ty_->ActualTy());
      
    }cnt++;
  }
  return new tr::ExpAndTy(nullptr, type::IntTy::Instance());
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---SubscriptVar---"<<std::endl;

  tr::ExpAndTy *var = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *subscript = subscript_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *exp_mem=new tr::ExExp(new tree::MemExp(
    new tree::BinopExp(tree::PLUS_OP,
      var->exp_->UnEx(),
      new tree::BinopExp(tree::MUL_OP,
        subscript->exp_->UnEx(),
        new tree::ConstExp(8)))));
  return new tr::ExpAndTy(exp_mem,((type::ArrayTy*)var->ty_)->ty_->ActualTy());
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---VarExp---"<<std::endl;

  return var_->Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---NilExp---"<<std::endl;

  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(0)),type::NilTy::Instance());
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---IntExp---"<<std::endl;

  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(val_)),type::IntTy::Instance());
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---StringExp---"<<std::endl;

  auto *lib=temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(lib, str_));
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(lib)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---CallExp---"<<std::endl;

  auto *elist=new tree::ExpList();
  for(auto *x : args_->GetList()){
    tree::Exp *e=x->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
    elist->Append(e);
  }
  env::FunEntry *entry = (env::FunEntry *)venv->Look(func_);
  if(!entry->level_->parent_){
    tr::Exp *e = new tr::ExExp(frame::ExternalCall(func_->Name(), elist));
    return new tr::ExpAndTy(e, entry->result_);
  }
  tree::Exp *staticlink = new tree::TempExp(reg_manager->FramePointer());
  while(level != entry->level_->parent_){
    staticlink = level->frame_->formals_.front()->ToExp(staticlink);
    level = level->parent_;
  }
  elist->Insert(staticlink);
  tr::Exp *e= new tr::ExExp(new tree::CallExp(new tree::NameExp(entry->label_), elist));
  return new tr::ExpAndTy(e, entry->result_);

  /* End for lab5 code */

}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---OpExp---"<<std::endl;

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
    case LT_OP:{
      cj=new tree::CjumpStm(tree::RelOp(tree::LT_OP),l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    }

    case LE_OP:{    
      cj=new tree::CjumpStm(tree::RelOp::LE_OP,l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case GT_OP:
{      cj=new tree::CjumpStm(tree::RelOp::GT_OP,l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case GE_OP:
      {cj=new tree::CjumpStm(tree::RelOp::GE_OP,l,r,nullptr,nullptr);
      temp::Label** trues=&cj->true_label_;
      temp::Label** falses=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(trues,falses,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case EQ_OP:
      {if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      }
      else{
        cj=new tree::CjumpStm(tree::RelOp(tree::EQ_OP),l,r,nullptr,nullptr);
        temp::Label** trues=&cj->true_label_;
        temp::Label** falses=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(trues,falses,cj);
        return new tr::ExpAndTy(exp, type::IntTy::Instance());
      }}
    case NEQ_OP:
      {if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      }
      else{
        cj=new tree::CjumpStm(tree::RelOp(tree::NE_OP),l,r,nullptr,nullptr);
        temp::Label** trues=&cj->true_label_;
        temp::Label** falses=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(trues,falses,cj);
        return new tr::ExpAndTy(exp, type::IntTy::Instance());
      }}
  }

}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---RecordExp---"<<std::endl;

  auto *elist=new tree::ExpList();
  for (auto *x : fields_->GetList()){
    tree::Exp *e=x->exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
    elist->Append(e);
  }
  auto *exp = new tree::ExpList();
  exp->Append(new tree::ConstExp(elist->GetList().size()*8));
  
  auto *t=temp::TempFactory::NewTemp();
  tree::Stm *stm = new tree::MoveStm(new tree::TempExp(t), frame::ExternalCall("alloc_record", exp));
  int cnt = 0;
  for(auto exp : elist->GetList()){
    stm = new tree::SeqStm(stm, new tree::MoveStm(
      new tree::MemExp(new tree::BinopExp(tree::PLUS_OP, new tree::TempExp(t), new tree::ConstExp(8*cnt))), exp));
    cnt++;
  }
  return new tr::ExpAndTy(new tr::ExExp(new tree::EseqExp(stm, new tree::TempExp(t))), tenv->Look(typ_)->ActualTy());
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---SeqExp---"<<std::endl;

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
  std::cout<<"---AssignExp---"<<std::endl;

  tree::Exp *ET_var = var_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tree::Exp *ET_exp = exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tr::Exp *exp=new tr::NxExp(new tree::MoveStm(ET_var,ET_exp));
  return new tr::ExpAndTy(exp, type::VoidTy::Instance()); 

}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---IfExp---"<<std::endl;


  tr::ExpAndTy *testExpTy = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *thenExpTy = then_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *translatedExp;


   // 获取条件表达
  tr::Cx testCx = testExpTy->exp_->UnCx(errormsg);
  
  temp::Label *trueLabel = temp::LabelFactory::NewLabel();
  temp::Label *falseLabel = temp::LabelFactory::NewLabel();
  temp::Label *mergeLabel = temp::LabelFactory::NewLabel();
  temp::Temp *resultTemp = temp::TempFactory::NewTemp();
  *testCx.trues_ = trueLabel;
  *testCx.falses_ = falseLabel;
  

  if (elsee_) {
    tr::ExpAndTy *elseExpTy = elsee_->Translate(venv, tenv, level, label, errormsg);

    // 检查then和else的类型是否匹配
      std::vector<temp::Label *> *jumpTargets = new std::vector<temp::Label *>{mergeLabel};

      translatedExp = new tr::ExExp(new tree::EseqExp(
        testCx.stm_,
        new tree::EseqExp(
          new tree::LabelStm(trueLabel),
          new tree::EseqExp(
            new tree::MoveStm(new tree::TempExp(resultTemp), thenExpTy->exp_->UnEx()),
            new tree::EseqExp(
              new tree::JumpStm(new tree::NameExp(mergeLabel), jumpTargets),
              new tree::EseqExp(
                new tree::LabelStm(falseLabel),
                new tree::EseqExp(
                  new tree::MoveStm(new tree::TempExp(resultTemp), elseExpTy->exp_->UnEx()),
                  new tree::EseqExp(
                    new tree::JumpStm(new tree::NameExp(mergeLabel), jumpTargets),
                    new tree::EseqExp(new tree::LabelStm(mergeLabel), new tree::TempExp(resultTemp))
                  )
                )
              )
            )
          )
        )
      ));
    
  } else {

    translatedExp = new tr::NxExp(new tree::SeqStm(
      testCx.stm_,
      new tree::SeqStm(
        new tree::LabelStm(trueLabel),
        new tree::SeqStm(thenExpTy->exp_->UnNx(), new tree::LabelStm(falseLabel))
      )
    ));
  }

  return new tr::ExpAndTy(translatedExp, thenExpTy->ty_->ActualTy());

}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---WhileExp---"<<std::endl;

  auto *tst_translated=test_->Translate(venv, tenv, level, label, errormsg);
  temp::Label *d_l = temp::LabelFactory::NewLabel();
  tr::ExpAndTy *body_translated = body_->Translate(venv, tenv, level, d_l, errormsg);
  temp::Label *b_l = temp::LabelFactory::NewLabel();
  temp::Label *t_l = temp::LabelFactory::NewLabel();
  auto cx=tst_translated->exp_->UnCx(errormsg);
  *(cx.trues_)=b_l;
  *(cx.falses_)=d_l;
  std::vector<temp::Label *> *j = new std::vector<temp::Label *>{t_l};
  tr::Exp *exp=new tr::NxExp(
      new tree::SeqStm(new tree::LabelStm(t_l),
        new tree::SeqStm(cx.stm_,
          new tree::SeqStm(new tree::LabelStm(b_l),
            new tree::SeqStm(body_translated->exp_->UnNx(),
                new tree::SeqStm(new tree::JumpStm(new tree::NameExp(t_l),j),
                  new tree::LabelStm(d_l)
                )
              )
            )
          )
        )
  );
  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---ForExp---"<<std::endl;

  // 翻译循环的下界和上界
  tr::ExpAndTy *lo_translated = lo_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *hi_translated = hi_->Translate(venv, tenv, level, label, errormsg);
  tr::Access *access=tr::Access::AllocLocal(level,escape_);
  // venv->BeginScope();
  venv->Enter(var_,new env::VarEntry(access,lo_translated->ty_,true));
  // venv->EndScope();
  temp::Label *d_l = temp::LabelFactory::NewLabel();
  tr::ExpAndTy *body_translated = body_->Translate(venv, tenv, level, d_l, errormsg);

  
  temp::Label *b_l = temp::LabelFactory::NewLabel();
  temp::Label *t_l = temp::LabelFactory::NewLabel();
  std::vector<temp::Label *> *j = new std::vector<temp::Label *>{t_l};
  
  tree::Exp *fr_exp = access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer()));

  tr::Exp *exp=new tr::NxExp(
    new tree::SeqStm(new tree::MoveStm(fr_exp,lo_translated->exp_->UnEx()),
      new tree::SeqStm(new tree::LabelStm(t_l),
        new tree::SeqStm(new tree::CjumpStm(tree::LE_OP,fr_exp,hi_translated->exp_->UnEx(),b_l,d_l),
          new tree::SeqStm(new tree::LabelStm(b_l),
            new tree::SeqStm(body_translated->exp_->UnNx(),
              new tree::SeqStm(new tree::MoveStm(fr_exp, new tree::BinopExp(tree::BinOp::PLUS_OP, fr_exp, new tree::ConstExp(1))),
                new tree::SeqStm(new tree::JumpStm(new tree::NameExp(t_l),j),
                  new tree::LabelStm(d_l)
                )
              )
            )
          )
        )
      )
    )
  );
  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---BreakExp---"<<std::endl;

  std::vector<temp::Label *> *j = new std::vector<temp::Label *>;
  j->push_back(label);
  auto *stm=new tree::JumpStm(new tree::NameExp(label), j);
  return new tr::ExpAndTy(new tr::NxExp(stm), type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---LetExp---"<<std::endl;

  // tree::SeqStm *stmlist=nullptr;
  tree::Stm *stm=nullptr;
  tree::Exp *exp = nullptr;
  venv->BeginScope();
  tenv->BeginScope();
  for(auto x:decs_->GetList()){
    if(!stm){
      stm = x->Translate(venv, tenv, level, label, errormsg)->UnNx();
    }
    else{
      stm = new tree::SeqStm(stm, x->Translate(venv, tenv, level, label, errormsg)->UnNx());
    }
  }
    tr::ExpAndTy *let_body=body_->Translate(venv, tenv, level, label, errormsg);
    if(!stm){
      exp = let_body->exp_->UnEx();
    }
    else{
      exp = new tree::EseqExp(stm, let_body->exp_->UnEx());
    }
  venv->EndScope();
  tenv->EndScope();


  return new tr::ExpAndTy(new tr::ExExp(exp),let_body->ty_->ActualTy());

}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---ArrayExp---"<<std::endl;

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
  tr::Exp *exp = new tr::ExExp(frame::ExternalCall("init_array", args));
  return new tr::ExpAndTy(exp,tenv->Look(typ_)->ActualTy()); 
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---VoidExp---"<<std::endl;

  return new tr::ExpAndTy(nullptr, type::VoidTy::Instance());
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---FunctionDec---"<<std::endl;
  
  // 第一步：处理函数头部，注册函数
  for(auto *x : functions_->GetList()){
    auto f_ty=x->params_->MakeFormalTyList(tenv,errormsg);
    std::list<bool>f_escape;
    int count = 1;
    for(auto *y:x->params_->GetList()){
        if (count > 6) {
            y->escape_ = true;
        }
      f_escape.push_back(y->escape_);
    }
    f_escape.push_front(true);//静态链?
    frame::Frame *newFrame=frame::NewFrame(x->name_,f_escape);
    tr::Level *newLevel=new tr::Level(newFrame,level);

    type::Ty *res_ty;
    if(x->result_){
      res_ty=tenv->Look(x->result_);
    }
    else{
      res_ty=type::VoidTy::Instance();
    }
    venv->Enter(x->name_,new env::FunEntry(newLevel, x->name_, f_ty, res_ty));
  }
  // 第二步：处理函数体
  for(auto *x : functions_->GetList()){
    venv->BeginScope();
    auto *fentry=(env::FunEntry*)venv->Look(x->name_);
    
    
    // 跳过静态链（第一个参数）
    auto a_i = (fentry->level_->frame_->formals_).begin();
    auto p_i = x->params_->GetList().begin();
    auto f_i = fentry->formals_->GetList().begin();
    a_i++;
    for (; p_i!= x->params_->GetList().end(); ++p_i, ++f_i, ++a_i) {
      venv->Enter((*p_i)->name_, new env::VarEntry(new tr::Access(fentry->level_, *a_i), *f_i));
    }

    tr::ExpAndTy *body_et = x->body_->Translate(venv, tenv, fentry->level_, fentry->label_, errormsg);
    venv->EndScope();
    //创建返回值赋值语句并处理函数出口
    auto exp1 =new tree::TempExp(reg_manager->ReturnValue());
    auto exp2= body_et->exp_->UnEx();
    tree::Stm *stm = new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()), body_et->exp_->UnEx());
    stm = frame::ProcEntryExit1(fentry->level_->frame_, stm);
    frags->PushBack(new frame::ProcFrag(stm, fentry->level_->frame_));
  }
  return new tr::ExExp(new tree::ConstExp(0));
}


tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---VarDec---"<<std::endl;

  auto *exp_ty=init_->Translate(venv, tenv, level, label, errormsg);
  // 将变量名和新创建的变量入口添加到变量环境中
  tr::Access * v_access=tr::Access::AllocLocal(level,escape_);
  venv->Enter(var_, new env::VarEntry(v_access, exp_ty->ty_));

  //将MoveStm节点包装在一个NxExp
  return new tr::NxExp(new tree::MoveStm(v_access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer())),exp_ty->exp_->UnEx()));
}


tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---TypeDec---"<<std::endl;

  auto type_list = types_->GetList();
  for (auto *x : type_list) 
    tenv->Enter(x->name_, new type::NameTy(x->name_, nullptr));
  for (auto *x  : type_list) {
    type::NameTy *namety = (type::NameTy *) tenv->Look(x->name_);
    namety->ty_ = x->ty_->Translate(tenv, errormsg);
  }
  return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---NameTy---"<<std::endl;

  //return new type::NameTy(name_,tenv->Look(name_));
  type::Ty *ty = tenv->Look(name_);
  return ty;
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---RecordTy---"<<std::endl;

  auto record_list = record_->MakeFieldList(tenv,errormsg);
  return new type::RecordTy(record_list);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---ArrayTy---"<<std::endl;

  auto ty = tenv->Look(array_);
  return new type::ArrayTy(ty);
}

} // namespace absyn啊啊啊啊好难啊啊啊啊难死了

```







```c++
#include "tiger/codegen/codegen.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include <string_view>

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

void saveregs(assem::InstrList *list,std::vector<temp::Temp *> tmp_saved){
  for(auto reg: reg_manager->CalleeSaves()->GetList()){
    temp::Temp *tmp = temp::TempFactory::NewTemp();
    tmp_saved.push_back(tmp);
    list->Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(tmp), new temp::TempList(reg)));
  }
}

void restoreregs(assem::InstrList *list,std::vector<temp::Temp *> tmp_saved){
  int i = 0;
  for(auto reg: reg_manager->CalleeSaves()->GetList()){
    list->Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(reg), new temp::TempList(tmp_saved[i])));
  }
}

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----Codegen-----"<<std::endl;
  fs_= frame_->name_->Name() + "_framesize";
  auto *list = new assem::InstrList();
  std::vector<temp::Temp *> tmp_saved;
  //saveregs(list,tmp_saved)
  for(auto x: reg_manager->CalleeSaves()->GetList()){
    temp::Temp *tmp = temp::TempFactory::NewTemp();
    tmp_saved.push_back(tmp);
    list->Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(tmp), new temp::TempList(x)));
  }
  
  for (auto stm : traces_->GetStmList()->GetList())
    stm->Munch(*list,fs_);
  
  //restoreregs(list,tmp_saved)
  int i = 0;
  for(auto x: reg_manager->CalleeSaves()->GetList()){
    list->Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(x), new temp::TempList(tmp_saved[i++])));
  }

  //std::unique_ptr<AssemInstr> assem_instr_;
  assem_instr_ = std::make_unique<AssemInstr>(list);

  frame::ProcEntryExit2(assem_instr_->GetInstrList());
  /* End for lab5 code */
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {

/* TODO: Put your lab5 code here */
/**
 * Generate code for passing arguments
 * @param args argument list
 * @param instr_holder instruction holder
 * @return temp list to hold arguments
 */
temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  std::cout<<"-----ExpList-----"<<std::endl;

  temp::Temp *ret=temp::TempFactory::NewTemp();
  auto *tplist=new temp::TempList();
  for(auto x :exp_list_){
    temp::Temp *local_ret=x->Munch(instr_list,fs);
    tplist->Append(local_ret);
    if(tplist->GetList().size()<=6){
      instr_list.Append(new assem::MoveInstr(
          "movq `s0, `d0" , 
          new temp::TempList(reg_manager->ArgRegs()->NthTemp(tplist->GetList().size()-1)), 
          new temp::TempList(local_ret)
          ));
    }
    else{
      auto sp=reg_manager->StackPointer();
      instr_list.Append(new assem::OperInstr(
        "subq $8, %rsp", 
        new temp::TempList(sp), 
        nullptr, 
        nullptr));
      instr_list.Append(new assem::OperInstr(
        "movq `s0, (%rsp)",
        new temp::TempList(sp), 
        new temp::TempList(local_ret), 
        nullptr));

    }
  }
  return tplist;
  /* End for lab5 code */
}

void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----SeqStm-----"<<std::endl;

  // SeqStm should not exist in codegen phase
  left_->Munch(instr_list,fs);
  right_->Munch(instr_list,fs);
  /* End for lab5 code */
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----LabelStm-----"<<std::endl;

  instr_list.Append(new assem::LabelInstr(label_->Name(),label_));
  
  /* End for lab5 code */
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----JumpStm-----"<<std::endl;

  instr_list.Append(new assem::OperInstr("jmp " + exp_->name_->Name(), nullptr, nullptr, new assem::Targets(jumps_)));
  /* End for lab5 code */
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----CjumpStm-----"<<std::endl;

  auto *right=right_->Munch(instr_list,fs);
  auto *left=left_->Munch(instr_list,fs);
  instr_list.Append(new assem::OperInstr("cmpq `s0, `s1" , nullptr,new temp::TempList({right,left}),nullptr));
  std::string s_asm="";
  switch (op_)
  {
  case EQ_OP:
    s_asm+="je";
    break;
  case NE_OP:
    s_asm+="jne";
    break;
  case LT_OP:
    s_asm+="jl";
    break;
  case LE_OP:
    s_asm+="jle";
    break;
  case GT_OP:
    s_asm+="jg";
    break;
  case GE_OP:
    s_asm+="jge";
    break;
  default:
    break;
  }      
  s_asm+=" `j0";
  instr_list.Append(new assem::OperInstr(s_asm , nullptr, nullptr,  new assem::Targets(new std::vector<temp::Label *>{true_label_})));
  /* End for lab5 code */
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----MoveStm-----"<<std::endl;

  // if (typeid(*dst_) == typeid(MemExp)) {
  //   MemExp* dst_mem = static_cast<MemExp*>(dst_);
  //   if (typeid(*dst_mem->exp_) == typeid(BinopExp)) {
  //     BinopExp* dst_binop = static_cast<BinopExp *>(dst_mem->exp_);
  //     if(dst_binop->op_ == tree::BinOp::PLUS_OP && 
  //         typeid(*dst_binop->right_) == typeid(ConstExp)) {
  //       Exp *e1 = dst_binop->left_; Exp *e2 = src_;
  //       /*MOVE(MEM(e1+i), e2) */
  //       instr_list.Append(new assem::MoveInstr(
  //         "movq `s0, " + std::to_string(static_cast<tree::ConstExp *>(dst_binop->right_)->consti_) + "(`s1)" , 
  //         nullptr, 
  //         new temp::TempList({(e2->Munch(instr_list,fs)),(e1->Munch(instr_list,fs))})
  //         ));

  //     }
  //     else if(dst_binop->op_== PLUS_OP &&
  //       typeid(*dst_binop->left_) == typeid(ConstExp)) {
  //       Exp *e1 = dst_binop->right_; Exp *e2 = src_;
  //       /*MOVE(MEM(i+e1), e2) */
  //       instr_list.Append(new assem::MoveInstr(
  //         "movq `s0, " + std::to_string(static_cast<tree::ConstExp *>(dst_binop->left_)->consti_) + "(`s1)" , 
  //         nullptr, 
  //         new temp::TempList({(e2->Munch(instr_list,fs)),(e1->Munch(instr_list,fs))})
  //         ));
  //     }
  //     else if(typeid(*src_) == typeid(MemExp)) {
  //       MemExp *src_mem = static_cast<MemExp*>(src_);
  //       Exp *e1=dst_mem->exp_, *e2=src_mem->exp_;
  //       /*MOVE(MEM(e1), MEM(e2)) */
  //       temp::Temp *tmp0=temp::TempFactory::NewTemp();
  //       temp::Temp *tmp1=e1->Munch(instr_list,fs); 
  //       temp::Temp *tmp2=e2->Munch(instr_list,fs); 
  //       instr_list.Append(new assem::MoveInstr(
  //         "movq (`s0), `s1" , 
  //         nullptr, 
  //         new temp::TempList({tmp2,tmp0})
  //         ));
  //       instr_list.Append(new assem::MoveInstr(
  //         "movq `s0, (`s1)" , 
  //         nullptr, 
  //         new temp::TempList({tmp0,tmp1})
  //         ));
  //     }
  //     else {
  //       Exp *e1=dst_mem->exp_, *e2=src_;
  //       /*MOVE(MEM(e1), e2) */
  //       temp::Temp *tmp1=e1->Munch(instr_list,fs); 
  //       temp::Temp *tmp2=e2->Munch(instr_list,fs); 
  //       instr_list.Append(new assem::MoveInstr(
  //         "movq `s0, (`s1)" , 
  //         nullptr, 
  //         new temp::TempList({tmp2,tmp1})
  //         ));
  //     }
  //   }
  //   else if (typeid(*dst_) == typeid(TempExp)) {
  //   Exp *e2=src_;
  //   /*MOVE(TEMP~i, e2) */
  //   temp::Temp *tmp1=dst_->Munch(instr_list,fs); 
  //   temp::Temp *tmp2=e2->Munch(instr_list,fs); 
  //   instr_list.Append(new assem::MoveInstr(
  //         "movq `s0, `s1" , 
  //         nullptr, 
  //         new temp::TempList({static_cast<tree::TempExp *>(dst_)->temp_,tmp2})
  //         ));
  //   }
  // }
  if(typeid(*dst_) == typeid(tree::MemExp)){

    auto s=src_->Munch(instr_list, fs);
    auto d=((tree::MemExp*)dst_)->exp_->Munch(instr_list, fs);

    auto *src = new temp::TempList({s,d});
    instr_list.Append(new assem::OperInstr("movq `s0, (`s1)", nullptr, src, nullptr));
    
  }
  else{
    auto s=src_->Munch(instr_list, fs);
    auto d=dst_->Munch(instr_list, fs);
    auto *src = new temp::TempList({s});
    auto *dst = new temp::TempList({d});
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", dst, src));
    
  }



  /* End for lab5 code */
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----ExpStm-----"<<std::endl;

  exp_->Munch(instr_list,fs);
  /* End for lab5 code */
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----BinopExp-----"<<std::endl;

  temp::Temp *l=left_->Munch(instr_list,fs); 
  temp::Temp *r=right_->Munch(instr_list,fs); 

  temp::Temp *rdx = reg_manager->Registers()->NthTemp(3);
  temp::Temp *rax = reg_manager->ReturnValue();

  temp::Temp *ret=temp::TempFactory::NewTemp();
  switch (op_)
  {
  case PLUS_OP:
    instr_list.Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(ret), new temp::TempList(l)));
    instr_list.Append(new assem::OperInstr(std::string("addq `s0, `d0"), new temp::TempList(ret), new temp::TempList({r,ret}), nullptr));
    break;
  case MINUS_OP:
    instr_list.Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(ret), new temp::TempList(l)));
    instr_list.Append(new assem::OperInstr(std::string("subq `s0, `d0"), new temp::TempList(ret), new temp::TempList({r,ret}), nullptr));
    break;
  case MUL_OP:
    instr_list.Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(rax), new temp::TempList(l)));
    instr_list.Append(new assem::OperInstr("cqto", new temp::TempList({rax,rdx}), new temp::TempList({rax}), nullptr));
    instr_list.Append(new assem::OperInstr("imulq `s0", new temp::TempList({rax,rdx}),new temp::TempList({r,rax,rdx}), nullptr));
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList({ret}),new temp::TempList({rax})));
    break;
  case DIV_OP:
    instr_list.Append(new assem::MoveInstr(std::string("movq `s0, `d0"), new temp::TempList(rax), new temp::TempList(l)));
    instr_list.Append(new assem::OperInstr("cqto", new temp::TempList({rax,rdx}), new temp::TempList({rax}), nullptr));
    instr_list.Append(new assem::OperInstr("idivq `s0", new temp::TempList({rax,rdx}),new temp::TempList({r,rax,rdx}), nullptr));
    instr_list.Append(new assem::MoveInstr("movq `s0, `d0", new temp::TempList({ret}),new temp::TempList({rax})));
    break;
  
  default:
    break;
  }
  return ret;
  /* End for lab5 code */
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----MemExp-----"<<std::endl;

  temp::Temp *ret=temp::TempFactory::NewTemp();
  temp::Temp *exp=exp_->Munch(instr_list,fs); 
  instr_list.Append(new assem::MoveInstr(
          "movq (`s0), `d0" , 
          new temp::TempList({ret}), 
          new temp::TempList({exp})
          ));
  return ret;
  /* End for lab5 code */
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----TempExp-----"<<std::endl;

  if(temp_!=reg_manager->FramePointer())
  {
    return temp_;
  }
  else{
    temp::Temp *reg = temp::TempFactory::NewTemp();
    instr_list.Append(new assem::OperInstr("leaq "+std::string(fs)+"(`s0), `d0", new temp::TempList(reg), new temp::TempList(reg_manager->StackPointer()), nullptr));
    return reg;
  }
  /* End for lab5 code */
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----EseqExp-----"<<std::endl;

  // EseqExp should not exist in codegen phase
  return nullptr;
  /* End for lab5 code */
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----NameExp-----"<<std::endl;

  std::string s_asm="leaq "+name_->Name()+"(%rip), `d0";
  temp::Temp *ret=temp::TempFactory::NewTemp();
  instr_list.Append(new assem::OperInstr(
    "leaq "+name_->Name()+"(%rip), `d0" , 
    new temp::TempList({ret}), 
    nullptr, 
    nullptr));
  return ret;
  /* End for lab5 code */
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----ConstExp-----"<<std::endl;

  std::string s_asm="movq $"+std::to_string(consti_)+", `d0";
  temp::Temp *ret=temp::TempFactory::NewTemp();
  instr_list.Append(new assem::MoveInstr(s_asm , new temp::TempList({ret}), nullptr));
  return ret;
  /* End for lab5 code */
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::cout<<"-----CallExp-----"<<std::endl;
  auto *tmp_list=args_->MunchArgs(instr_list, fs);
  temp::Temp *ret=temp::TempFactory::NewTemp();
  instr_list.Append(new assem::OperInstr("call "+((tree::NameExp *)fun_)->name_->Name() , reg_manager->CallerSaves(),reg_manager->ArgRegs(), nullptr));
  int tmp_num = tmp_list->GetList().size();
  if(tmp_num-6>0){
    std::string instr = "addq $" + std::to_string((tmp_num-6)*8)+", `d0";
    instr_list.Append(new assem::OperInstr(instr, new temp::TempList(reg_manager->StackPointer()), nullptr, nullptr));
  }

  instr_list.Append(new assem::MoveInstr("movq `s0, `d0" , new temp::TempList({ret}), new temp::TempList({reg_manager->ReturnValue()})));

  return ret;
  /* End for lab5 code */
}

} // namespace tree还有高手？

```



# semant



```c++

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
    return type::VoidTy::Instance();
  }
  type::Ty *ty_th = then_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(elsee_){
    type::Ty *ty_el = elsee_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
      if(ty_th->IsSameType(ty_el)){
        return ty_th;
      }
      errormsg->Error(pos_, "then exp and else exp type mismatch");
      return type::VoidTy::Instance();
  }
  else{
    if(typeid(*ty_th)!=typeid(type::VoidTy)){
      errormsg->Error(pos_, "if-then exp's body must produce no value");
      return type::VoidTy::Instance();
    }
    else return ty_th;
  }
  return type::VoidTy::Instance();
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty_test = test_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*ty_test)!=typeid(type::IntTy)){
    errormsg->Error(pos_, "test_ required");
    return type::IntTy::Instance();
  }
  type::Ty *ty_bd = body_->SemAnalyze(venv, tenv, labelcount+1, errormsg)->ActualTy();
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
  venv->Enter(var_, new env::VarEntry(ty_lo, true));
  body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);
  venv->EndScope();
  return type::VoidTy::Instance(); 
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if(labelcount==0)
    errormsg->Error(pos_, "loop variable can't be assigned");
  return type::VoidTy::Instance(); 
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
```

