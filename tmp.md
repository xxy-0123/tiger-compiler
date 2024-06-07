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

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  frame::Access *frameAccess = level->frame_->AllocLocal(escape);
  return new Access(level, frameAccess);
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

// expression
class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    //printf("ExExp UnEx");
    return exp_;
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    //printf("ExExp UnNx");
    return new tree::ExpStm(UnEx());
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    //printf("begin ExExp UnCx");
    tree::CjumpStm* stm=new tree::CjumpStm(tree::NE_OP,exp_,new tree::ConstExp(0),nullptr,nullptr);
    temp::Label** trues=&stm->true_label_;
    temp::Label** falses=&stm->false_label_;

    return Cx(trues,falses,stm);
  }
};

// non-value expression
class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    //printf("NxExp UnEx");
    return new tree::EseqExp(stm_, new tree::ConstExp(0));
  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    //printf("NxExp UnNx");
    return stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    //printf("NxExp UnCx");
    // errormsg->Error(1, "wafwdada");
    return Cx(nullptr, nullptr, nullptr);
  }
};

//conditional expression
class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
      : cx_(trues, falses, stm) {}
  
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    //printf("begin NxExp UnEx");
    temp::Temp *r = temp::TempFactory::NewTemp();
    temp::Label *t = temp::LabelFactory::NewLabel();
    temp::Label *f = temp::LabelFactory::NewLabel();
    *(cx_.trues_)=t;
    *(cx_.falses_)=f;
    return new tree::EseqExp(
      new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(1)),
        new tree::EseqExp(cx_.stm_, 
          new tree::EseqExp(new tree::LabelStm(f),
            new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r),new tree::ConstExp(0)),
              new tree::EseqExp(new tree::LabelStm(t), new tree::TempExp(r))))));
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    //printf("CxExp UnNx");
    temp::Label* label=temp::LabelFactory::NewLabel();
    *(cx_.trues_)=label;
    *(cx_.falses_)=label;
    return new tree::SeqStm(cx_.stm_,new tree::LabelStm(label)); // convert to exp without return value
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    //printf("CxExp UnCx");
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  temp::Label *main_label_ = temp::LabelFactory::NamedLabel("tigermain");
  std::list<bool> escapes;
  tr::Level *level = new tr::Level(frame::NewFrame(main_label_, std::list<bool>()), nullptr);
  main_level_ .reset(level);
  tenv_ = std::make_unique<env::TEnv>();
  venv_ = std::make_unique<env::VEnv>();
  // fill after main level init!
  FillBaseTEnv();
  FillBaseVEnv();
  tr::ExpAndTy *tree = absyn_tree_.get()->Translate(venv_.get(), tenv_.get(), level, main_label_, errormsg_.get());
  tree::Stm *tmp = new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()), tree->exp_->UnEx());
  tmp = frame::ProcEntryExit1(level->frame_, tmp);
  
  frags->PushBack(new frame::ProcFrag(tmp, level->frame_));
}

tr::Level *newLevel(Level *parent, temp::Label *name, std::list<bool> formals){
  formals.push_front(true);
  frame::Frame *newFrame = frame::NewFrame(name, formals);
  tr::Level *level = new tr::Level(newFrame, parent);

  return level;
}

tr::Level *outermost(){
  temp::Label *label = temp::LabelFactory::NamedLabel(std::string("tigermain"));
  std::list<bool> formals;
  Level *level = newLevel(nullptr, label, formals);

  return level;
}

} // namespace tr

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---AbsynTree---"<<std::endl;
  return root_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  // errormsg->Error(pos_, "begin simple var and sym is %s", sym_->Name().data());
  //std::cout<<std::endl<<"---SimpleVar(ok)---"<<std::endl;
  auto *ventry = (env::VarEntry*)venv->Look(sym_);
  tree::Exp *exp = new tree::TempExp(reg_manager->FramePointer());
  while(ventry->access_->level_!=level){
    exp=level->frame_->formals_.front()->ToExp(exp);
    level=level->parent_;
  }
  return new tr::ExpAndTy(new tr::ExExp(ventry->access_->access_->ToExp(exp)),ventry->ty_->ActualTy());
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---FieldVar---"<<std::endl;
  tr::ExpAndTy* var = var_->Translate(venv, tenv, level, label, errormsg);
  if(typeid(*(var->ty_)) != typeid(type::RecordTy)){
    errormsg->Error(this->pos_, "not a record type");
  }
  std::list<type::Field*> field_list = (static_cast<type::RecordTy*>(var->ty_))->fields_->GetList();
  std::list<type::Field *>::iterator tmp = field_list.begin();
  int offset=0;
  while(tmp != field_list.end()){
    //errormsg->Error(pos_, "parse a field %s", (*tmp)->name_->Name().data());
    if((*tmp)->name_->Name() == sym_->Name()){
      tr::Exp *filedVar = new tr::ExExp(new tree::MemExp(
        new tree::BinopExp(tree::PLUS_OP, var->exp_->UnEx(), new tree::ConstExp(offset * reg_manager->WordSize()))));
      return new tr::ExpAndTy(filedVar, (*tmp)->ty_->ActualTy());
    }
    tmp++;
    offset++;
  }
  return new tr::ExpAndTy(nullptr, type::IntTy::Instance());
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---SubscriptVar---"<<std::endl;
  tr::ExpAndTy *var = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *subscript = subscript_->Translate(venv, tenv, level, label, errormsg);
  if(typeid(var->ty_) != typeid(type::ArrayTy)){
    // errormsg->Error(pos_, "array type required");
  }
  if(typeid(((type::ArrayTy*)var->ty_)->ty_->ActualTy()) != typeid(type::RecordTy)){
    // errormsg->Error(pos_, "not a record");
  }
  tr::Exp *subscriptVar = new tr::ExExp(
    new tree::MemExp(new tree::BinopExp(tree::PLUS_OP, var->exp_->UnEx(), 
      new tree::BinopExp(tree::MUL_OP, subscript->exp_->UnEx(), new tree::ConstExp(reg_manager->WordSize())))));
  return new tr::ExpAndTy(subscriptVar, ((type::ArrayTy*)var->ty_)->ty_->ActualTy());
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---VarExp(ok)---"<<std::endl;
  return var_->Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //errormsg->Error(pos_, "begin nil exp");
  std::cout<<std::endl<<"---NilExp---"<<std::endl;
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(0)), type::NilTy::Instance());
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //printf("begin int exp %d", val_);
  //std::cout<<std::endl<<"---IntExp(ok)---"<<std::endl;
  return new tr::ExpAndTy(new tr::ExExp(new tree::ConstExp(val_)), type::IntTy::Instance());
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---StringExp---"<<std::endl;
  temp::Label *label_tmp = temp::LabelFactory::NewLabel();
  frame::StringFrag *str_frag = new frame::StringFrag(label_tmp, str_);
  frags->PushBack(str_frag);
  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(label_tmp)), type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---CallExp(ok)---"<<std::endl;
  tree::ExpList *elist=new tree::ExpList();
  for(auto *x : args_->GetList()){
    tree::Exp *e=x->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
    elist->Append(e);
  }
  env::FunEntry *entry = (env::FunEntry *)venv->Look(func_);
  if(!entry->level_->parent_){
    tr::Exp *e = new tr::ExExp(new tree::CallExp(new tree::NameExp(
    temp::LabelFactory::NamedLabel(temp::LabelFactory::LabelString(func_))), elist));
    return new tr::ExpAndTy(e, entry->result_);
  }
  tree::Exp *staticlink = new tree::TempExp(reg_manager->FramePointer());
  while(level != entry->level_->parent_){
    staticlink = level->frame_->formals_.front()->ToExp(staticlink);
    level = level->parent_;
  }
  elist->Append(staticlink);
  tr::Exp *e= new tr::ExExp(new tree::CallExp(new tree::NameExp(entry->label_), elist));
  return new tr::ExpAndTy(e, entry->result_);

}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---OpExp(ok)---"<<std::endl;
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
  std::cout<<std::endl<<"---RecordExp---"<<std::endl;
  //errormsg->Error(pos_, "begin record exp %s", typ_->Name().data());
  type::Ty *typ_ty = tenv->Look(typ_)->ActualTy();
  if(!typ_ty){
    //errormsg->Error(pos_, "undefined type %s", typ_->Name().data());
  }
  tree::ExpList *expList = new tree::ExpList();
  std::list<type::Field *> fieldsList = ((type::RecordTy *)typ_ty)->fields_->GetList();
  std::list<absyn::EField *> eFieldList = fields_->GetList();
  std::list<type::Field *>::iterator field_it = fieldsList.begin();
  std::list<absyn::EField *>::iterator eField_it = eFieldList.begin();
  for(; field_it != fieldsList.end() && eField_it != eFieldList.end(); field_it++){
    tr::ExpAndTy *tmp = (*eField_it)->exp_->Translate(venv, tenv, level, label, errormsg);
    expList->Append(tmp->exp_->UnEx());
    eField_it++;
  }
  temp::Temp *reg = temp::TempFactory::NewTemp();
  int size = expList->GetList().size();
  tree::ExpList *exps = new tree::ExpList();
  exps->Append(new tree::ConstExp(size * reg_manager->WordSize()));
  tree::Stm *stm = new tree::MoveStm(new tree::TempExp(reg), frame::ExternalCall(std::string("alloc_record"), exps));
  int count = 0;
  for(auto exp : expList->GetList()){
    stm = new tree::SeqStm(stm, new tree::MoveStm(
      new tree::MemExp(new tree::BinopExp(tree::PLUS_OP, new tree::TempExp(reg), 
        new tree::ConstExp(count * reg_manager->WordSize()))), exp));
    count++;
  }

  return new tr::ExpAndTy(new tr::ExExp(new tree::EseqExp(stm, new tree::TempExp(reg))), typ_ty->ActualTy());
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---SeqExp(ok)---"<<std::endl;
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
  //std::cout<<std::endl<<"---AssignExp(ok)---"<<std::endl;
  tree::Exp *ET_var = var_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tree::Exp *ET_exp = exp_->Translate(venv,tenv,level,label,errormsg)->exp_->UnEx();
  tr::Exp *exp=new tr::NxExp(new tree::MoveStm(ET_var,ET_exp));
  return new tr::ExpAndTy(exp, type::VoidTy::Instance()); 
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---IfExp(ok)---"<<std::endl;
  tr::ExpAndTy *testExpTy = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *thenExpTy = then_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *translatedExp;


   // 获取条件表达
  tr::Cx cx = testExpTy->exp_->UnCx(errormsg);
  temp::Label *trueLabel = temp::LabelFactory::NewLabel();
  temp::Label *falseLabel = temp::LabelFactory::NewLabel();
  temp::Label *mergeLabel = temp::LabelFactory::NewLabel();
  temp::Temp *resultTemp = temp::TempFactory::NewTemp();
  *cx.trues_ = trueLabel;
  *cx.falses_ = falseLabel;

  if (elsee_) {
    tr::ExpAndTy *elseExpTy = elsee_->Translate(venv, tenv, level, label, errormsg);

    // 检查then和else的类型是否匹配
      std::vector<temp::Label *> *j = new std::vector<temp::Label *>{mergeLabel};

      translatedExp = new tr::ExExp(new tree::EseqExp(
        cx.stm_,
        new tree::EseqExp(
          new tree::LabelStm(trueLabel),
          new tree::EseqExp(
            new tree::MoveStm(new tree::TempExp(resultTemp), thenExpTy->exp_->UnEx()),
            new tree::EseqExp(
              new tree::JumpStm(new tree::NameExp(mergeLabel), j),
              new tree::EseqExp(
                new tree::LabelStm(falseLabel),
                new tree::EseqExp(
                  new tree::MoveStm(new tree::TempExp(resultTemp), elseExpTy->exp_->UnEx()),
                  new tree::EseqExp(
                    new tree::JumpStm(new tree::NameExp(mergeLabel), j),
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
      cx.stm_,
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
  //std::cout<<std::endl<<"---WhileExp(ok)---"<<std::endl;
  auto *tst_translated=test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *body_translated = body_->Translate(venv, tenv, level, label, errormsg);
  temp::Label *b_l = temp::LabelFactory::NewLabel();
  temp::Label *t_l = temp::LabelFactory::NewLabel();
  temp::Label *d_l = temp::LabelFactory::NewLabel();
  auto cx=tst_translated->exp_->UnCx(errormsg);
  *(cx.trues_)=b_l;
  *(cx.falses_)=d_l;
  std::vector<temp::Label *> *j = new std::vector<temp::Label *>;
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
  std::cout<<std::endl<<"---ForExp---"<<std::endl;
  tr::ExpAndTy *lo = lo_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *hi = hi_->Translate(venv, tenv, level, label, errormsg);
  //errormsg->Error(hi_->pos_, "end high");
  tr::Exp *exp;
  if(!lo->ty_->IsSameType(type::IntTy::Instance())){
    // errormsg->Error(lo_->pos_, "lo for exp's range type is not integer");
  }
  if(!hi->ty_->IsSameType(type::IntTy::Instance())){
    // errormsg->Error(hi_->pos_, "hi for exp's range type is not integer");
  }
  tr::Access *access = tr::Access::AllocLocal(level, escape_);
  venv->Enter(var_, new env::VarEntry(access, lo->ty_, true));
  temp::Label *l_done = temp::LabelFactory().NewLabel();
  tr::ExpAndTy *body = body_->Translate(venv, tenv, level, l_done, errormsg);
  tree::Exp *frameExp = access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer()));

  temp::Label *l_test = temp::LabelFactory().NewLabel();
  temp::Label *l_body = temp::LabelFactory().NewLabel();
  std::vector<temp::Label *> *jumps = new std::vector<temp::Label *>;
  jumps->push_back(l_test);

  exp = new tr::NxExp(new tree::SeqStm(new tree::MoveStm(frameExp, lo->exp_->UnEx()), 
    new tree::SeqStm(new tree::LabelStm(l_test),
      new tree::SeqStm(new tree::CjumpStm(tree::LE_OP, frameExp, hi->exp_->UnEx(), l_body, l_done),
        new tree::SeqStm(new tree::LabelStm(l_body),
          new tree::SeqStm(body->exp_->UnNx(),
                new tree::SeqStm(new tree::MoveStm(frameExp, new tree::BinopExp(tree::BinOp::PLUS_OP, frameExp, new tree::ConstExp(1))),
                  new tree::SeqStm(new tree::JumpStm(new tree::NameExp(l_test), jumps),
                    new tree::LabelStm(l_done)))))))));
    

  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---BreakExp---"<<std::endl;
  std::vector<temp::Label *> *jumps = new std::vector<temp::Label *>;
  jumps->push_back(label);
  tr::Exp *exp = new tr::NxExp(new tree::JumpStm(new tree::NameExp(label), jumps));

  return new tr::ExpAndTy(exp, type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---LetExp(ok)---"<<std::endl;
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
  std::cout<<std::endl<<"---ArrayExp---"<<std::endl;
  type::Ty *typ_ty = tenv->Look(typ_)->ActualTy();
  tr::ExpAndTy *size = size_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *init = init_->Translate(venv, tenv, level, label, errormsg);
  std::initializer_list<tree::Exp *> list = {size->exp_->UnEx(), init->exp_->UnEx()};
  tree::ExpList *expList = new tree::ExpList(list);
  tr::Exp *exp = new tr::ExExp(
    new tree::CallExp(new tree::NameExp(temp::LabelFactory::NamedLabel("init_array")), expList));

  return new tr::ExpAndTy(exp, typ_ty->ActualTy());
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---VoidExp---"<<std::endl;
  return new tr::ExpAndTy(nullptr, type::VoidTy::Instance());
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---FunctionDec---"<<std::endl;
  std::list<FunDec *> functions = functions_->GetList();
  for(FunDec *function : functions){
    //errormsg->Error(pos_, "do first");
    type::TyList *formals = function->params_->MakeFormalTyList(tenv, errormsg);
    std::list<bool> formal_escape;
    std::list<Field *> fieldList = function->params_->GetList();
    for(Field * param : fieldList){
      formal_escape.push_back(param->escape_);
    }
    tr::Level *new_level = tr::newLevel(level, function->name_, formal_escape);
    if(function->result_){
      type::Ty *result_ty = tenv->Look(function->result_);
      //printf("enter the function %s\n", function->name_->Name().data());
      venv->Enter(function->name_, new env::FunEntry(new_level, function->name_, formals, result_ty));
    }
    else{
      //printf("enter the function %s\n", function->name_->Name().data());
      venv->Enter(function->name_, new env::FunEntry(new_level, function->name_, formals, type::VoidTy::Instance()));
    }
  }

  for(FunDec *function : functions){
    env::EnvEntry *fun = venv->Look(function->name_);
    type::TyList *formals = ((env::FunEntry*)fun)->formals_;
    auto formal_it = formals->GetList().begin();
    auto param_it = (function->params_->GetList()).begin();
    venv->BeginScope();
    std::list<frame::Access*> accessList = ((env::FunEntry*)fun)->level_->frame_->formals_;
    std::list<frame::Access*>::iterator access_it = accessList.begin();
    // Be careful with this!!!!! Skip the staticlink!
    access_it++;
    for(; param_it != (function->params_->GetList()).end(); param_it++){
      venv->Enter((*param_it)->name_, 
        new env::VarEntry(new tr::Access(((env::FunEntry*)fun)->level_, *access_it), *formal_it));
      formal_it++;
      access_it++;
    }
    tr::ExpAndTy *body = function->body_->Translate(venv, tenv, ((env::FunEntry*)fun)->level_, ((env::FunEntry*)fun)->label_, errormsg);

    if(((env::FunEntry *)fun)->result_->ActualTy()->IsSameType(type::VoidTy::Instance())){
      if(!body->ty_->IsSameType(type::VoidTy::Instance())){
        //errormsg->Error(pos_, "procedure returns value");
      }
      else{
        //errormsg->Error(pos_, "did right procedure returns value");
      }
    }
    else{
      //errormsg->Error(pos_, "should returns value");
    }
    venv->EndScope();
    tree::Stm *stm = new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()), body->exp_->UnEx());
    stm = frame::ProcEntryExit1(((env::FunEntry *)fun)->level_->frame_, stm);
    frags->PushBack(new frame::ProcFrag(stm, ((env::FunEntry *)fun)->level_->frame_));
  }

  return new tr::ExExp(new tree::ConstExp(0));
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //std::cout<<std::endl<<"---VarDec(ok)---"<<std::endl;
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
  std::cout<<std::endl<<"---TypeDec---"<<std::endl;
  std::list<NameAndTy*> nameAndTyList = types_->GetList();
  /* At the first time, parse the header and write it into tenv */
  for(auto &nameAndTy : nameAndTyList){
    int count = 0;
    for(auto &tmp : nameAndTyList){
      if(nameAndTy->name_ == tmp->name_){
        count++;
      }
      if(count >= 2){
        //errormsg->Error(pos_, "two types have the same name");
        return nullptr;
      }
    }
    tenv->Enter(nameAndTy->name_, new type::NameTy(nameAndTy->name_, nullptr));
  }
  /* At the second time, parse the body with the tenv */
  for(auto &nameAndTy : nameAndTyList){
    type::NameTy *type = static_cast<type::NameTy*>(tenv->Look(nameAndTy->name_));
    type->ty_ = nameAndTy->ty_->Translate(tenv, errormsg);
    if(typeid(*(type->ty_)) == typeid(type::RecordTy)){
      //errormsg->Error(pos_, "woc is record");
    }
    tenv->Set(nameAndTy->name_, type);
  }
  for(auto &nameAndTy : nameAndTyList){
    type::Ty *name_ty = tenv->Look(nameAndTy->name_);
    type::Ty *tmp = name_ty;
    while(typeid(*tmp) == typeid(type::NameTy)){
      tmp = ((type::NameTy *)tmp)->ty_;
      if(((type::NameTy *)tmp) == ((type::NameTy *)name_ty)){
        //errormsg->Error(pos_, "illegal type cycle");
        return nullptr;
      }
    }
  }
  //errormsg->Error(pos_, "end typedec");

  return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---NameTy---"<<std::endl;
  //errormsg->Error(pos_, "begin namety");
  type::Ty *name_ty = tenv->Look(name_);
  if(!name_ty){
    //errormsg->Error(pos_, "should be defined type");
    return type::VoidTy::Instance();
  }
  return new type::NameTy(name_, name_ty);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---RecordTy---"<<std::endl;
  type::FieldList *fields = record_->MakeFieldList(tenv, errormsg);
  
  return new type::RecordTy(fields);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::cout<<std::endl<<"---ArrayTy---"<<std::endl;
  type::Ty *array_ty = tenv->Look(array_);
  if(!array_ty){
    //errormsg->Error(pos_, "should be defined type!");
    return type::VoidTy::Instance();
  }
  return new type::ArrayTy(array_ty);
}

} // namespace absyn

```

