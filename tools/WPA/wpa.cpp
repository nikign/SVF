//===- wpa.cpp -- Whole program analysis -------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===-----------------------------------------------------------------------===//

/*
 // Whole Program Pointer Analysis
 //
 // Author: Yulei Sui,
 */

#include <SVF-FE/LLVMUtil.h>
#include <WPA/WPAPass.h>
#include <Util/BasicTypes.h>
#include <llvm/ADT/GraphTraits.h>

using namespace llvm;
using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string> InputFilename(cl::Positional,
        llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));


uint64_t getBBCounter(const llvm::BasicBlock *BB) {

  if(BB==NULL){
      printf("\nBBConst is null\n");
      return(-2);
  }

  printf("\nBB pointer: %p ---\n", BB);

  MDNode* BBid_meta = NULL;
  uint64_t block_counter;

  if (BB->size() == 0)
  {
      printf("There is a basic block with no instructions in the program.\n");
      exit(1);
  }
  if (BB->getInstList().empty()){
      printf("BB inst list was empty.\n");
      return(-5);
  }

  for (const Instruction& instr : BB->getInstList()) {
        if (!instr.hasMetadata())
        {
            printf("The first instruction of the block should include block id, have you instrumented the code with set-counter-BBid-llvm-pass first?");
            continue;
        }
        BBid_meta = instr.getMetadata("BBid");
        break;
  }

  if(!BBid_meta){
    printf("\nSome block did not have ID in it\n");
    return(-3);
  }

  std::string meta_string = cast<MDString>(BBid_meta->getOperand(0))->getString();

  block_counter = std::strtoull(meta_string.c_str(), NULL, 16);

  printf("\n block id: %llu\n", block_counter);

  return(block_counter);
}


int main(int argc, char ** argv)
{

    int arg_num = 0;
    char **arg_value = new char*[argc];
    std::vector<std::string> moduleNameVec;
    SVFUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Whole Program Points-to Analysis\n");

    SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);

    WPAPass *wpa = new WPAPass();
    wpa->runOnModule(svfModule);

    ICFG *G = wpa->getPointerAnalysis()->getPAG()->getICFG();
    for (const auto Node : nodes<ICFG*>(G)){
        const llvm::BasicBlock *Node_bb = Node->getBB();
        getBBCounter(Node_bb);
    }
    const std::string file_name = "icfg_custom_simple";
    G->dump(file_name, true);

    return 0;
}
