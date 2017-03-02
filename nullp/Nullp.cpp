#include <vector>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/InstVisitor.h"
using namespace llvm;

namespace {
    struct NullpPass : public FunctionPass {
        //Structure to keep track of which Values we've already processed
        SmallSet<Value*, 64> ptrValues;

        static char ID;
        NullpPass() : FunctionPass(ID) {}

        /**
         * helper function to print the contents of ptrValues
         */
        void dumpPtrValues() {
            errs() << " ---- ptrValues contains: \n";
            for(auto V :ptrValues) {
                errs() << V->getName() << " - " << (*V) << "\n";
            }
        }


        /**
         * InstVisitor that performs relevant checks for instructions involving pointers
         */
        struct PtrInstVisitor : public InstVisitor<PtrInstVisitor> {
            std::vector<Instruction*> loadBuffer;

            PtrInstVisitor() {}

            void visitStoreInst(StoreInst &SI) 
            {
                auto pType = isa<ConstantPointerNull>(SI.getValueOperand());
                if(!pType) {
                    SI.getValueOperand()->dump();
                    if(loadBuffer.size() != 0) {
                        loadBuffer.clear();
                    }
                }
            }

            void visitLoadInst(LoadInst &LI)
            {
                loadBuffer.push_back(&(cast<Instruction>(LI)));
            }

        };



        /**
         * Extract all pointer values and add them to ptrValues
         */
        void extractPointerValues(Function &F)
        {
            //loop over all instructions
            for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
                errs() << *I << "\n";

                if(auto alloca = dyn_cast<AllocaInst>(&(*I))) {
                    if(isa<PointerType>(alloca->getAllocatedType())) {
                        ptrValues.insert(alloca);
                        errs() << "\n use chain for " << alloca << "\n";
                        for(auto U = alloca->user_begin(), E = alloca->user_end(); U != E; ++U) {
                            (*U)->dump();
                        }
                        errs() << "\n";
                    }
                }
            }
        }

        void findBadPtrUsage()
        {
            for(auto V: ptrValues) {

                //strategy:
                //loop backwards over all references,
                //keep track of a vector of current load instructions
                //if find store (!null value), then throw away vector and continue
                //if at end, the load instructions vector is nonempty then these are uses of uninitialized pointer or null pointer

                PtrInstVisitor PIV;
                for(auto U : V->users()){
                    if(auto ins = dyn_cast<Instruction>(U)) {
                        PIV.visit(ins);
                    } else {
                        errs() << "not an instruction:\n";
                        errs() << (*U) << "\n";
                    }
                }


                if(PIV.loadBuffer.size() != 0) {
                    for(auto ins : PIV.loadBuffer) {
                        errs() << "ERROR: reading from a null pointer at ";
                        if(ins->getDebugLoc()) {
                            //If clang is run with -g flag, print line number
                            errs() << "line number " << ins->getDebugLoc().getLine() << ", col" << ins->getDebugLoc().getCol()  << "\n";
                        }
                        else {
                            //otherwise, just display the address location of the error
                            errs() << ins << "\n";
                        }
                        ins->dump();
                        errs() << ins->getName() << "\n";
                    }
                } 
            }
        }

        /**
         * Function pass main code
         */
        virtual bool runOnFunction(Function &F) {
            ptrValues.clear(); //make sure we're not using leftovers from a previous function
            errs() << "Considering function " << F.getName() << "!\n";

            ///Step 1: Find all ptrValues that are pointers
            extractPointerValues(F);

            dumpPtrValues();

            errs() <<"\n ---------- STEP 2 \n";
            findBadPtrUsage();

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
