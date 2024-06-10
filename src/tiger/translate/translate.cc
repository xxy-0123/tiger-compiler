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
  auto *main_label_ = temp::LabelFactory::NamedLabel("tigermain");
  auto *exp_ty = absyn_tree_->Translate(venv_.get(), tenv_.get(), main_level_.get(), main_label_, errormsg_.get());
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
  {
    tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
    while(ventry->access_->level_!=level){
      exp=level->frame_->formals_.front()->ToExp(exp);
      level=level->parent_;
    }

    return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());}
  else
  {
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
  
  auto *var = var_->Translate(venv, tenv, level, label, errormsg);
  int cnt=0;
  for(auto *x:((type::RecordTy*)var->ty_)->fields_->GetList()){
    if(x->name_==sym_){
      tr::Exp *exp = new tr::ExExp(
        new tree::MemExp(
          new tree::BinopExp(
            tree::PLUS_OP,
            new tree::ConstExp(cnt*8),
            var->exp_->UnEx())));
      return new tr::ExpAndTy(exp, x->ty_->ActualTy());
    }
    cnt++;
  }
  return new tr::ExpAndTy(nullptr, type::IntTy::Instance());
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---SubscriptVar---"<<std::endl;

  auto *var = var_->Translate(venv, tenv, level, label, errormsg);
  auto *subscript = subscript_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *exp=new tr::ExExp(new tree::MemExp(
    new tree::BinopExp(tree::PLUS_OP,
      var->exp_->UnEx(),
      new tree::BinopExp(tree::MUL_OP,
        new tree::ConstExp(8),
        subscript->exp_->UnEx()))));
  return new tr::ExpAndTy(exp,((type::ArrayTy*)var->ty_)->ty_->ActualTy());
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

  auto *lb=temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(lb, str_));
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(lb)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---CallExp---"<<std::endl;
  //翻译参数
  auto *elist=new tree::ExpList();
  for(auto *x : args_->GetList()){
    tree::Exp *e=x->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
    elist->Append(e);
  }
  auto *entry = (env::FunEntry *)venv->Look(func_);
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
      cj=new tree::CjumpStm(tree::LT_OP,l,r,nullptr,nullptr);
      temp::Label** t=&cj->true_label_;
      temp::Label** f=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(t,f,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());
    }

    case LE_OP:{    
      cj=new tree::CjumpStm(tree::LE_OP,l,r,nullptr,nullptr);
      temp::Label** t=&cj->true_label_;
      temp::Label** f=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(t,f,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case GT_OP:
{      cj=new tree::CjumpStm(tree::GT_OP,l,r,nullptr,nullptr);
      temp::Label** t=&cj->true_label_;
      temp::Label** f=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(t,f,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case GE_OP:
      {cj=new tree::CjumpStm(tree::GE_OP,l,r,nullptr,nullptr);
      temp::Label** t=&cj->true_label_;
      temp::Label** f=&cj->false_label_;
      tr::Exp *exp= new tr::CxExp(t,f,cj);
      return new tr::ExpAndTy(exp, type::IntTy::Instance());}
    case EQ_OP:
    {
      // {if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      // }
      // else{
        cj=new tree::CjumpStm(tree::EQ_OP,l,r,nullptr,nullptr);
        temp::Label** t=&cj->true_label_;
        temp::Label** f=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(t,f,cj);
        return new tr::ExpAndTy(exp, type::IntTy::Instance());
      //  }
      }
    case NEQ_OP:
      {if(typeid(left_->Translate(venv,tenv,level,label,errormsg)->ty_) == typeid(type::StringTy)) {

      }
      else{
        cj=new tree::CjumpStm(tree::RelOp(tree::NE_OP),l,r,nullptr,nullptr);
        temp::Label** t=&cj->true_label_;
        temp::Label** f=&cj->false_label_;
        tr::Exp *exp= new tr::CxExp(t,f,cj);
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
    auto *e=x->exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
    elist->Append(e);
  }
  auto *exp = new tree::ExpList();
  exp->Append(new tree::ConstExp(elist->GetList().size()*8));
  
  auto *t=temp::TempFactory::NewTemp();
  tree::Stm *stm = new tree::MoveStm(new tree::TempExp(t), frame::ExternalCall("alloc_record", exp));
  int cnt = 0;
  for(auto exp : elist->GetList()){
    stm = new tree::SeqStm(stm, new tree::MoveStm(
      new tree::MemExp(
        new tree::BinopExp(
          tree::PLUS_OP, 
          new tree::TempExp(t), 
          new tree::ConstExp(8*cnt)
        )
      ),
    exp));
    cnt++;
  }
  return new tr::ExpAndTy(new tr::ExExp(new tree::EseqExp(stm, new tree::TempExp(t))), tenv->Look(typ_)->ActualTy());
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---SeqExp---"<<std::endl;

  tr::Exp *exp=nullptr;//!exp必须初始化
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

  auto *varet = var_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  auto *expet = exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tr::Exp *exp=new tr::NxExp(new tree::MoveStm(varet,expet));
  return new tr::ExpAndTy(exp, type::VoidTy::Instance()); 

}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---IfExp---"<<std::endl;


  auto *test_et = test_->Translate(venv, tenv, level, label, errormsg);
  auto *then_et = then_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *translated_exp;


   // 获取条件表达
  auto testcx = test_et->exp_->UnCx(errormsg);
  
  auto *tl = temp::LabelFactory::NewLabel();
  auto *fl = temp::LabelFactory::NewLabel();
  auto *ml = temp::LabelFactory::NewLabel();
  auto *re_tmp = temp::TempFactory::NewTemp();
  *testcx.trues_ = tl;
  *testcx.falses_ = fl;
  

  if (elsee_) {
    auto *else_et = elsee_->Translate(venv, tenv, level, label, errormsg);

    // 检查then和else的类型是否匹配
      auto *j = new std::vector<temp::Label *>{ml};

      translated_exp = new tr::ExExp(new tree::EseqExp(
        testcx.stm_,
        new tree::EseqExp(
          new tree::LabelStm(tl),
          new tree::EseqExp(
            new tree::MoveStm(new tree::TempExp(re_tmp), then_et->exp_->UnEx()),
            new tree::EseqExp(
              new tree::JumpStm(new tree::NameExp(ml), j),
              new tree::EseqExp(
                new tree::LabelStm(fl),
                new tree::EseqExp(
                  new tree::MoveStm(new tree::TempExp(re_tmp), else_et->exp_->UnEx()),
                  new tree::EseqExp(
                    new tree::JumpStm(new tree::NameExp(ml), j),
                    new tree::EseqExp(new tree::LabelStm(ml), new tree::TempExp(re_tmp))
                  )
                )
              )
            )
          )
        )
      ));
    
  } else {

    translated_exp = new tr::NxExp(new tree::SeqStm(
      testcx.stm_,
      new tree::SeqStm(
        new tree::LabelStm(tl),
        new tree::SeqStm(then_et->exp_->UnNx(), new tree::LabelStm(fl))
      )
    ));
  }

  return new tr::ExpAndTy(translated_exp, then_et->ty_->ActualTy());

}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<"---WhileExp---"<<std::endl;

  auto *tst_translated=test_->Translate(venv, tenv, level, label, errormsg);
  auto *d_l = temp::LabelFactory::NewLabel();
  auto *body_translated = body_->Translate(venv, tenv, level, d_l, errormsg);
  auto *b_l = temp::LabelFactory::NewLabel();
  auto *t_l = temp::LabelFactory::NewLabel();
  auto cx=tst_translated->exp_->UnCx(errormsg);
  *(cx.trues_)=b_l;
  *(cx.falses_)=d_l;
  auto *j = new std::vector<temp::Label *>{t_l};
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
  auto *lo = lo_->Translate(venv, tenv, level, label, errormsg);
  auto *hi = hi_->Translate(venv, tenv, level, label, errormsg);
  auto *access=tr::Access::AllocLocal(level,escape_);
  // venv->BeginScope();
  venv->Enter(var_,new env::VarEntry(access,lo->ty_,true));
  // venv->EndScope();
  auto *d_l = temp::LabelFactory::NewLabel();
  auto *body = body_->Translate(venv, tenv, level, d_l, errormsg);

  
  auto *b_l = temp::LabelFactory::NewLabel();
  auto *t_l = temp::LabelFactory::NewLabel();
  auto *j = new std::vector<temp::Label *>{t_l};
  
  auto *fr_exp = access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer()));

  tr::Exp *exp=new tr::NxExp(
    new tree::SeqStm(new tree::MoveStm(fr_exp,lo->exp_->UnEx()),
      new tree::SeqStm(new tree::LabelStm(t_l),
        new tree::SeqStm(new tree::CjumpStm(tree::LE_OP,fr_exp,hi->exp_->UnEx(),b_l,d_l),
          new tree::SeqStm(new tree::LabelStm(b_l),
            new tree::SeqStm(body->exp_->UnNx(),
              new tree::SeqStm(new tree::MoveStm(fr_exp, new tree::BinopExp(tree::BinOp::PLUS_OP,new tree::ConstExp(1), fr_exp)),
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

  auto *j = new std::vector<temp::Label *>{label};
  // j->push_back(label);
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
    auto *let_body=body_->Translate(venv, tenv, level, label, errormsg);
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
  auto *arr_size=size_->Translate(venv, tenv, level, label, errormsg)->exp_->UnEx();
  auto *arr_init=init_->Translate(venv, tenv, level, label, errormsg)->exp_->UnEx();
  auto *args=new tree::ExpList();
  args->Append(arr_size);
  args->Append(arr_init);
  auto *exp = new tr::ExExp(frame::ExternalCall("init_array", args));
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
    std::list<bool>f_escape{true};
    int count = 1;
    for(auto *y:x->params_->GetList()){
        if (count > 6) {
            y->escape_ = true;
        }
      f_escape.push_back(y->escape_);
    }
    // f_escape.push_front(true);//静态链?
    auto *newFrame=frame::NewFrame(x->name_,f_escape);
    auto *newLevel=new tr::Level(newFrame,level);

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

    auto *body_et = x->body_->Translate(venv, tenv, fentry->level_, fentry->label_, errormsg);
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
  auto * v_access=tr::Access::AllocLocal(level,escape_);
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
  for(auto *x : type_list) 
    tenv->Enter(x->name_, new type::NameTy(x->name_, nullptr));
  for(auto *x  : type_list) {
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
