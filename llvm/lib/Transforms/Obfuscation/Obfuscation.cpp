#include "llvm/Transforms/Obfuscation/Obfuscation.h"
#include <iostream>

// PreservedAnalyses Obfuscation::run(Module &M, ModuleAnalysisManager & AM) {
//   // errs() << "----> in Obfuscation::run, EnableFlattening =" << EnableFlattening << ", module name: " << m.getName() << "\n";
//   errs() << "----> in Obfuscation::run, EnableFlattening = null" << ", module name: " << M.getName() << "\n";
//   return PreservedAnalyses::all();
// }

using namespace llvm;

// Begin Obfuscator Options
static cl::opt<uint64_t> AesSeed("aesSeed", cl::init(0x1337),
                                    cl::desc("seed for the PRNG"));
static cl::opt<bool> EnableAntiClassDump("enable-acdobf", cl::init(false),
                                         cl::NotHidden,
                                         cl::desc("Enable AntiClassDump."));
static cl::opt<bool>
    EnableBogusControlFlow("enable-bcfobf", cl::init(false), cl::NotHidden,
                           cl::desc("Enable BogusControlFlow."));
static cl::opt<bool> EnableFlattening("enable-cffobf", cl::init(false),
                                      cl::NotHidden,
                                      cl::desc("Enable Flattening."));
static cl::opt<bool>
    EnableBasicBlockSplit("enable-splitobf", cl::init(false), cl::NotHidden,
                          cl::desc("Enable BasicBlockSpliting."));
static cl::opt<bool>
    EnableSubstitution("enable-subobf", cl::init(false), cl::NotHidden,
                       cl::desc("Enable Instruction Substitution."));
static cl::opt<bool> EnableAllObfuscation("enable-allobf", cl::init(false),
                                          cl::NotHidden,
                                          cl::desc("Enable All Obfuscation."));
static cl::opt<bool> EnableFunctionCallObfuscate(
    "enable-fco", cl::init(false), cl::NotHidden,
    cl::desc("Enable Function CallSite Obfuscation."));
static cl::opt<bool>
    EnableStringEncryption("enable-strcry", cl::init(false), cl::NotHidden,
                           cl::desc("Enable Function CallSite Obfuscation."));
static cl::opt<bool>
    EnableIndirectBranching("enable-indibran", cl::init(false), cl::NotHidden,
                            cl::desc("Enable Indirect Branching."));
static cl::opt<bool>
    EnableFunctionWrapper("enable-funcwra", cl::init(false), cl::NotHidden,
                          cl::desc("Enable Function Wrapper."));
// End Obfuscator Options

PreservedAnalyses Obfuscation::run(Module &M, ModuleAnalysisManager & AM) {
  if (AesSeed!=0x1337) {
    cryptoutils->prng_seed(AesSeed);
  }
  else{
    cryptoutils->prng_seed();
  }

  // Initial ACD Pass
  if (EnableAllObfuscation || EnableAntiClassDump) {
    ModulePass *P = createAntiClassDumpPass();
    P->doInitialization(M);
    P->runOnModule(M);
    delete P;
  }

  // Now do FCO
  FunctionPass *FP = createFunctionCallObfuscatePass(
      EnableAllObfuscation || EnableFunctionCallObfuscate);
  FP->doInitialization(M);
  for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
    Function &F = *iter;
    if (!F.isDeclaration()) {
      FP->runOnFunction(F);
    }
  }
  delete FP;

  // Now Encrypt Strings
  ModulePass *MP = createStringEncryptionPass(EnableAllObfuscation ||
                                  EnableStringEncryption);
  MP->runOnModule(M);
  delete MP;

  /*
  // Placing FW here does provide the most obfuscation however the compile
  time
  // and product size would be literally unbearable for any large project
  // Move it to post run
  if (EnableAllObfuscation || EnableFunctionWrapper) {
    ModulePass *P = createFunctionWrapperPass();
    P->runOnModule(M);
    delete P;
  }*/
  // Now perform Function-Level Obfuscation
  for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
    Function &F = *iter;
    if (!F.isDeclaration()) {
      FunctionPass *P = NULL;
      P = createSplitBasicBlockPass(EnableAllObfuscation ||
                                    EnableBasicBlockSplit);
      P->runOnFunction(F);
      delete P;
      P = createBogusControlFlowPass(EnableAllObfuscation ||
                                     EnableBogusControlFlow);
      P->runOnFunction(F);
      delete P;
      P = createFlatteningPass(EnableAllObfuscation || EnableFlattening);
      P->runOnFunction(F);
      delete P;
      P = createSubstitutionPass(EnableAllObfuscation || EnableSubstitution);
      P->runOnFunction(F);
      delete P;
    }
  }
  errs() << "Doing Post-Run Cleanup\n";
  FunctionPass *P = createIndirectBranchPass(EnableAllObfuscation ||
                                             EnableIndirectBranching);
  vector<Function *> funcs;
  for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
    funcs.push_back(&*iter);
  }
  for (Function *F : funcs) {
    P->runOnFunction(*F);
  }
  delete P;
  MP = createFunctionWrapperPass(EnableAllObfuscation ||
                                 EnableFunctionWrapper);
  MP->runOnModule(M);
  delete MP;
  // Cleanup Flags
  vector<Function *> toDelete;
  for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
    Function &F = *iter;
    if (F.isDeclaration() && F.hasName() &&
        F.getName().contains("hikari_")) {
      for (User *U : F.users()) {
        if (Instruction *Inst = dyn_cast<Instruction>(U)) {
          Inst->eraseFromParent();
        }
      }
      toDelete.push_back(&F);
    }
  }
  for (Function *F : toDelete) {
    F->eraseFromParent();
  }

  errs() << "Hikari Out\n";
  return PreservedAnalyses::all();
}