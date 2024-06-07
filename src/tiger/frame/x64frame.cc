#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {

X64RegManager::X64RegManager() : RegManager() {
    /* TODO: Put your lab5 code here */
    //void Enter(Temp *t, std::string *s);
    temp::Temp *temp_ = temp::TempFactory::NewTemp();
    regs_.push_back(temp_);
    temp_map_->Enter(temp_, new std::string ("%rax"));
}

temp::TempList *X64RegManager::Registers() {
    /* TODO: Put your lab5 code here */
    auto regs = new temp::TempList();
    for (int i = 0; i < REG_COUNT; ++i) {
        regs->Append(regs_[i]);
    }
    return regs;
}

temp::TempList *X64RegManager::ArgRegs() {
    /* TODO: Put your lab5 code here */
    auto regs = new temp::TempList();
    regs->Append(regs_[RDI]);
    regs->Append(regs_[RSI]);
    regs->Append(regs_[RDX]);
    regs->Append(regs_[RCX]);
    regs->Append(regs_[R8]);
    regs->Append(regs_[R9]);
    return regs;
}

temp::TempList *X64RegManager::CallerSaves() {
    /* TODO: Put your lab5 code here */
    auto regs = new temp::TempList();
    regs->Append(regs_[RAX]);
    regs->Append(regs_[RCX]);
    regs->Append(regs_[RDX]);
    regs->Append(regs_[RSI]);
    regs->Append(regs_[RDI]);
    regs->Append(regs_[R8]);
    regs->Append(regs_[R9]);
    regs->Append(regs_[R10]);
    regs->Append(regs_[R11]);
    return regs;
}

temp::TempList *X64RegManager::CalleeSaves() {
    /* TODO: Put your lab5 code here */
    auto regs = new temp::TempList();
    regs->Append(regs_[RBX]);
    regs->Append(regs_[RBP]);
    regs->Append(regs_[R12]);
    regs->Append(regs_[R13]);
    regs->Append(regs_[R14]);
    regs->Append(regs_[R15]);
    return regs;
}

temp::TempList *X64RegManager::ReturnSink() {
    /* TODO: Put your lab5 code here */
    auto regs = CalleeSaves();
    regs->Append(FramePointer());
    regs->Append(ReturnValue());
    return regs;
}

int X64RegManager::WordSize() { 
  /* TODO: Put your lab5 code here */ 
  return 8;
}

temp::Temp *X64RegManager::FramePointer() { return regs_[FP];/* TODO: Put your lab5 code here */ }

temp::Temp *X64RegManager::StackPointer() { return regs_[SP];/* TODO: Put your lab5 code here */ }

temp::Temp *X64RegManager::ReturnValue() { return regs_[RV];/* TODO: Put your lab5 code here */ }

class InFrameAccess : public Access {
  /* TODO: Put your lab5 code here */
public:
  int offset;

  explicit InFrameAccess(int offset) : offset(offset) { }
  tree::Exp *ToExp(tree::Exp *framePtr) const override{
    return new tree::MemExp(new tree::BinopExp(tree::PLUS_OP,framePtr,new tree::ConstExp(offset)));
  }
  /* End for lab5 code */
};


class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : reg(reg) { }
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override{
    return new tree::TempExp(reg);
  }

  /* End for lab5 code */
};

class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */
public:
  //int offset=0;
  X64Frame(temp::Label *name, std::list<frame::Access *> *formals)
       : Frame(8, 0, name, formals) {}

  [[nodiscard]] std::string GetLabel() const override { return name_->Name(); }
  [[nodiscard]] temp::Label *Name() const override { return name_; }
  [[nodiscard]] std::list<frame::Access *> *Formals() const override {
  //   return &formals_;
  }
  frame::Access *AllocLocal(bool escape) override {
    /* TODO: Put your lab5 code here */
    if(escape){
      auto infAccess= new frame::InFrameAccess(offset);
      offset-=8;
      // return new frame::InFrameAccess(offset);
      return infAccess;
    }
    else return new frame::InRegAccess(temp::TempFactory::NewTemp());
  }
  void AllocOutgoSpace(int size) override {
    /* TODO: Put your lab5 code here */
  }
  /* End for lab5 code */
};

/* TODO: Put your lab5 code here */

frame::Frame *NewFrame(temp::Label *name, std::list<bool> formals) {
    /* TODO: Put your lab5 code here */
  auto formals_list = new std::list<frame::Access*>();
  
  int offset = -8;
  for (bool escape : formals) {
    if (escape) {
      formals_list->push_back(new frame::InFrameAccess(offset));
      offset -= 8;
    } else {
      formals_list->push_back(new frame::InRegAccess(temp::TempFactory::NewTemp()));
    }
  }

  return new frame::X64Frame(name, formals_list);
}

tree::Exp *ExternalCall(std::string_view s, tree::ExpList *args) {
    /* TODO: Put your lab5 code here */
}


/* End for lab5 code */

tree::Stm *ProcEntryExit1(frame::Frame *frame, tree::Stm *stm) {

    printf("\n begin procEntryExit1\n");
}

assem::InstrList *ProcEntryExit2(assem::InstrList body) { return nullptr; }

assem::Proc *ProcEntryExit3(frame::Frame *frame, assem::InstrList *body) {
  return nullptr;
}

} // namespace frame
