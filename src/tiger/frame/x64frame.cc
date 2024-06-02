#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {

X64RegManager::X64RegManager() : RegManager() {
    /* TODO: Put your lab5 code here */
}

temp::TempList *X64RegManager::Registers() {
    /* TODO: Put your lab5 code here */
}

temp::TempList *X64RegManager::ArgRegs() {
    /* TODO: Put your lab5 code here */
}

temp::TempList *X64RegManager::CallerSaves() {
    /* TODO: Put your lab5 code here */
}

temp::TempList *X64RegManager::CalleeSaves() {
    /* TODO: Put your lab5 code here */
}

temp::TempList *X64RegManager::ReturnSink() {
    /* TODO: Put your lab5 code here */
}

int X64RegManager::WordSize() { 
  /* TODO: Put your lab5 code here */ 
  return 8;
}

temp::Temp *X64RegManager::FramePointer() { /* TODO: Put your lab5 code here */ }

temp::Temp *X64RegManager::StackPointer() { /* TODO: Put your lab5 code here */ }

temp::Temp *X64RegManager::ReturnValue() { /* TODO: Put your lab5 code here */ }

class InFrameAccess : public Access {
  /* TODO: Put your lab5 code here */
  
  /* End for lab5 code */
};


class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : reg(reg) {}
  /* TODO: Put your lab5 code here */
  return new tree::TempExp(reg);
  /* End for lab5 code */
};

class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */

  X64Frame(temp::Label *name, std::list<frame::Access *> *formals)
      : Frame(8, 0, name, formals), view_shift(nullptr) {}

  [[nodiscard]] std::string GetLabel() const override { return name_->Name(); }
  [[nodiscard]] temp::Label *Name() const override { return name_; }
  [[nodiscard]] std::list<frame::Access *> *Formals() const override {
    return formals_;
  }
  frame::Access *AllocLocal(bool escape) override {
    /* TODO: Put your lab5 code here */
    if(escape){
      offset-=8;
      // return new frame::InFrameAccess(offset);
      return new frame::InFrameAccess(offset);
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
}

tree::Exp *ExternalCall(std::string_view s, tree::ExpList *args) {
    /* TODO: Put your lab5 code here */
}


/* End for lab5 code */

} // namespace frame
