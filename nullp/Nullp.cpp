#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
using namespace llvm;

namespace {
  struct NullpPass : public FunctionPass {
    static char ID;
    NullpPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << "Considering function" << F.getName() << "!\n";

      //loop over all instructions
      for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
          errs() << *I << "\n";
          errs() << *I.getOpCodeName() << "\n";
          errs() << *I.isBinaryOp() << "\n";
      }

      return false; //Didn't modify anything
    }
  };
}

char NullpPass::ID = 0;

// Automatically enable the pass.
static void registerNullpPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new NullpPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerNullpPass);
