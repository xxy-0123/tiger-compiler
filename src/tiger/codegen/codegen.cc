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
  int num=exp_list_.size();
  auto *tplist=new temp::TempList();
  for(auto x :exp_list_){
    temp::Temp *local_ret=x->Munch(instr_list,fs);
    tplist->Append(local_ret);
    if(tplist->GetList().size()<=num){
      instr_list.Append(new assem::MoveInstr(
          "movq `s0, `d0" , 
          new temp::TempList(reg_manager->ArgRegs()->NthTemp(tplist->GetList().size()-1)), 
          new temp::TempList({local_ret})
          ));
    }
    else{
      std::string s_asm="movq `s0"+std::to_string((num-tplist->GetList().size())*8)+"%rsp";
      instr_list.Append(new assem::MoveInstr(s_asm , nullptr, new temp::TempList({local_ret})));

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
  args_->MunchArgs(instr_list, fs);
  std::string s_asm="call "+((tree::NameExp *)fun_)->name_->Name();
  temp::Temp *ret=temp::TempFactory::NewTemp();
  instr_list.Append(new assem::OperInstr(s_asm , reg_manager->CallerSaves(),reg_manager->ArgRegs(), nullptr));
  instr_list.Append(new assem::MoveInstr("movq `s0, `d0" , new temp::TempList({ret}), new temp::TempList({reg_manager->ReturnValue()})));

  return ret;
  /* End for lab5 code */
}

} // namespace tree还有高手？
