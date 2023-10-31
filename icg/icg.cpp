#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/CFG.h"
#include <iostream>
#include <string.h>

using namespace llvm;
using namespace std;

namespace {

struct icgPass : public PassInfoMixin<icgPass> {
  PreservedAnalyses run(Module &MOD, ModuleAnalysisManager &AM) {
    for (auto &F : MOD) {
	  Module *M = F.getParent();
      LLVMContext &context = F.getContext();
      string functionName = F.getName().str();
	  
	  // insert global varibale when first time running the module
	  Constant *initStr = ConstantDataArray::getString(context, "__ICG_PASS_INIT_STR__");
      string icfName = "indirect_call_flag";
	  string pfnName = "prev_function_name";
	  string zeroName = "zero_name";
	  // GlobalVariable
      GlobalVariable *icf = M->getGlobalVariable(icfName);
	  GlobalVariable *pfn = M->getGlobalVariable(pfnName);
	  GlobalVariable *zero = M->getGlobalVariable(zeroName);
	  
      if (!icf) {
        icf = new GlobalVariable(
            /*Module=*/*M,
            /*Type=*/Type::getInt32Ty(context),
            /*isConstant=*/false,
            /*Linkage=*/GlobalValue::CommonLinkage,
            /*Initializer=*/0,
            /*Name=*/icfName);
        icf->setInitializer(ConstantInt::get(Type::getInt32Ty(context), 0));
      }
	  
      if (!pfn) {
        pfn = new GlobalVariable(
            /*Module=*/*M,
            /*Type=*/initStr->getType(),
            /*isConstant=*/false,
            /*Linkage=*/GlobalValue::CommonLinkage,
            /*Initializer=*/initStr,
            /*Name=*/pfnName);
		pfn->setInitializer(initStr);
      }
	  
      if (!zero) {
        zero = new GlobalVariable(
            /*Module=*/*M,
            /*Type=*/Type::getInt32Ty(context),
            /*isConstant=*/false,
            /*Linkage=*/GlobalValue::CommonLinkage,
            /*Initializer=*/0,
            /*Name=*/zeroName);
		zero->setInitializer(ConstantInt::get(Type::getInt32Ty(context), 0));
      }
	  
	  
	  
	  FunctionType *printfType = FunctionType::get(Type::getInt32Ty(context), {Type::getInt8Ty(context)}, true);
	  FunctionCallee printfFunc = M->getOrInsertFunction("printf", printfType);
	  
	  // for indirect call 
      vector<Instruction *> set_flag_worklist;
	  
      for (auto &BB : F) {
        for (auto &I : BB) {
          if (CallInst *CI = dyn_cast<CallInst>(&I)) {
            Function *calledFunction = CI->getCalledFunction();
            if (!calledFunction) {
			  set_flag_worklist.push_back(CI);		  
            }
          }
        }
      }
	  
	  for (auto *indirectCallInst : set_flag_worklist) {
		  IRBuilder<> builder(context);
		  builder.SetInsertPoint(indirectCallInst);
		  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), 1), icf);
		  
	      Value *functionNameValue = builder.CreateGlobalStringPtr(functionName);
	      Value *formatString = builder.CreateGlobalStringPtr("\n__ICG_STDOUT__:%s");
		  builder.CreateCall(printfFunc, {formatString,functionNameValue});
	  }
	   vector<Instruction *> print_worklist;
	   
	   for (auto &BB : F){
	   	  Instruction* firstInstruction = BB.getFirstNonPHI();
		  if (firstInstruction){
		  	print_worklist.push_back(firstInstruction);
		  }
	   }
	   
	   for(auto *firstInst : print_worklist){
		   IRBuilder<> builder(context);
		   builder.SetInsertPoint(firstInst);
		   
		   LoadInst *icfValue = builder.CreateLoad(icf->getType(),icf,"icg_val");
		   LoadInst *zeroValue = builder.CreateLoad(zero->getType(),zero,"zero_val");
		   
		   Value *icmp = builder.CreateICmpSGT(icfValue, zeroValue, "cmp_val");
		   
		   // Value *debugString = builder.CreateGlobalStringPtr("\nDEBUG icmpResult=%d,icf=%d,zero=%d\n");
		   // builder.CreateCall(printfFunc, {debugString,icmp,icfValue,zeroValue});
		   
		   Instruction * thenInst = SplitBlockAndInsertIfThen(icmp,firstInst,false);
		   
		   builder.SetInsertPoint(thenInst);
		   
		   Value *functionNameValue = builder.CreateGlobalStringPtr(functionName);
		   Value *formatString = builder.CreateGlobalStringPtr("->%s\n");
		   builder.CreateCall(printfFunc, {formatString,functionNameValue,icfValue});
		   builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), 0), icf);
		   
		   builder.SetInsertPoint(firstInst);
		   
	   }
	     
    }
    return PreservedAnalyses::all();
  };
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {.APIVersion = LLVM_PLUGIN_API_VERSION,
          .PluginName = "icg pass",
          .PluginVersion = "v0.1",
          .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                  MPM.addPass(icgPass());
                });
          }};
}
