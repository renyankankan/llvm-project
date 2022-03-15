add_llvm_library(LLVMObfuscation
FunctionCallObfuscate.cpp
  CryptoUtils.cpp
  BogusControlFlow.cpp
  Substitution.cpp
  Flattening.cpp
  Utils.cpp
  SplitBasicBlocks.cpp
  AntiClassDump.cpp
  StringEncryption.cpp
  IndirectBranch.cpp
  FunctionWrapper.cpp
  Obfuscation.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Obfuscation

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  )