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
	  
	  // GlobalVariable
      GlobalVariable *icf = M->getGlobalVariable(icfName);
	  GlobalVariable *pfn = M->getGlobalVariable(pfnName);
	  
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
	  
	  
	  FunctionType *printfType = FunctionType::get(Type::getInt32Ty(context), {Type::getInt8Ty(context)}, true);
	  FunctionCallee printfFunc = M->getOrInsertFunction("printf", printfType);
	  
	  
      // Insert instructions to set global variable to 1 before the indirect
      // call. 
      for (auto &BB : F) {
        for (auto &I : BB) {
          if (CallInst *CI = dyn_cast<CallInst>(&I)) {
            Function *calledFunction = CI->getCalledFunction();
            if (!calledFunction) {

              // set the start point of the inserted instructions
              // set to 1	
			  IRBuilder<> ins_builder(context);
              ins_builder.SetInsertPoint(CI);
			  			  
              ins_builder.CreateStore(
                  ConstantInt::get(Type::getInt32Ty(context), 1), icf);
			  
			  // Print the head of the message.
			  Value *formatString = ins_builder.CreateGlobalStringPtr("\n__ICG_STDOUT__:"+functionName+"->");
			  ins_builder.CreateCall(printfFunc, {formatString});
			  
            }
          }
        }
      }
	  
      // I cannot insert following instructions to check global variable.
	  // Because the following operations require some basic block changes!
      /*
       *
       * if (*icf == 1){
       *   *icf = 0;
       *   printf(F.getName().str());
       * }
       *
       */
	  // So we will direct output all stuff to STDOUT.
	  // We will use "__ICG_STDOUT__:" defined in the code above to disdinguish
	  // the outputs from our LLVM PASS and original programme.
	  
	  // Why we need to scan whole BB in F? I have NO FUCKING IDEA WHY FOLLOWING CODE PARTS
	  // CANNOT PASS THE FUCKING IDIOT LLVM COMPLIER!!!!
	  
	  // FAILED VERSION ONE:
	  /*
	  
        	Instruction* firstInstruction = &F.front().front();
			IRBuilder<> print_builder(firstInstruction);
	  
	  */
	  
	  // FAILED VERSION TWO:
	  /*

  			Instruction* firstInstruction = GET VIA THE ENTRY OF FUNCTION
			IRBuilder<> print_builder(firstInstruction);

	  */

	  // That's why I have to use for loop
      for (auto &BB : F) {
		  	// Not very sure what *NonPHI* is
        	Instruction* firstInstruction = BB.getFirstNonPHI();
			IRBuilder<> print_builder(firstInstruction);
			Value *functionNameValue = print_builder.CreateGlobalStringPtr(functionName);
				
			Value *formatString = print_builder.CreateGlobalStringPtr("%s,I:%d\n");
			// Please note that we need to LOAD that variable to current stack first via its
			// pointer. Otherwise we cannot get its value.
			// Since old CreateLoad(icf) function has been depricated, we have to use
			// CreateLoad(icf->getType(),icf,"icg_val")
			LoadInst *icfValue = print_builder.CreateLoad(icf->getType(),icf,"icg_val");
			
			print_builder.CreateCall(printfFunc, {formatString,functionNameValue,icfValue});
			
			// set to 0
	        print_builder.CreateStore(
	            ConstantInt::get(Type::getInt32Ty(context), 0), icf);
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
