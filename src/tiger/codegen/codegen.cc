#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>
#include <string_view>

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  auto *list = new assem::InstrList();
  for (auto stm : traces_->GetStmList()->GetList())
    stm->Munch(*list);
  assem_instr_ =
  std::make_unique<AssemInstr>(frame::ProcEntryExit2(list));
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
  
  /* End for lab5 code */
}

void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  // SeqStm should not exist in codegen phase
  left_->Munch(instr_list,fs);
  right_->Munch(instr_list,fs);
  /* End for lab5 code */
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(new assem::LabelInstr(label_->Name(),label_));
  
  /* End for lab5 code */
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  instr_list.Append(new assem::OperInstr("jmp 'j0" , nullptr, nullptr, new assem::Targets(jumps_)));
  /* End for lab5 code */
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto right=right_->Munch(instr_list,fs);
  auto left=left_->Munch(instr_list,fs);
  instr_list.Append(new assem::OperInstr("cmpq 's0, 's1" , nullptr,new temp::TempList((right,left)),nullptr));
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
  s_asm+=" 'j0";
  instr_list.Append(new assem::OperInstr(s_asm , nullptr, nullptr,  new assem::Targets(new std::vector<temp::Label *>(true_label_,false_label_))));
  /* End for lab5 code */
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  if (typeid(*dst_) == typeid(MemExp)) {
    MemExp* dst_mem = static_cast<MemExp*>(dst_);
    if (typeid(*dst_mem->exp_) == typeid(BinopExp)) {
      BinopExp* dst_binop = static_cast<BinopExp *>(dst_mem->exp_);
      if(dst_binop->op_ == tree::BinOp::PLUS_OP && 
          typeid(*dst_binop->right_) == typeid(ConstExp)) {
        Exp *e1 = dst_binop->left_; Exp *e2 = src_;
        /*MOVE(MEM(e1+i), e2) */
        e1->Munch(instr_list,fs);
        e2->Munch(instr_list,fs);
        instr_list.Append(new assem::OperInstr(s_asm , nullptr, nullptr,  new assem::Targets(new std::vector<temp::Label *>(true_label_,false_label_))));

      }
      else if(dst_binop->op_== PLUS_OP &&
        typeid(*dst_binop->left_) == typeid(ConstExp)) {
        Exp *e1 = dst_binop->right_; Exp *e2 = src_;
        /*MOVE(MEM(i+e1), e2) */
        e1->Munch(il);
        e2->Munch(il);	
        il.emit(“STORE”);
      }
      else if(typeid(*src_) == typeid(MemExp)) {
        MemExp *src_mem = static_cast<MemExp*>(src_);
        Exp *e1=dst_mem->exp_, e2=src_mem->exp_;
        /*MOVE(MEM(e1), MEM(e2)) */
        e1->Munch(il); 
        e2->Munch(il); 
        il.emit(“MOVEM”);
      }
      else {
        Exp *e1=dst_mem->exp_, e2=src_;
        /*MOVE(MEM(e1), e2) */
        e1->Munch(il); 
        e2->Munch(il); 
        il.emit(“STORE”);
      }
    }
    else if (typeid(*dst_) == typeid(TempExp)) {
    Exp *e2=src_;
    /*MOVE(TEMP~i, e2) */
    e2->Munch(il); 
    il.emit(“ADD”);
  }





  /* End for lab5 code */
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  exp_->Munch(instr_list,fs);
  /* End for lab5 code */
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  // EseqExp should not exist in codegen phase
  
  /* End for lab5 code */
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  /* End for lab5 code */
}

} // namespace tree
