set(LLVM_LINK_COMPONENTS Support)

add_clang_executable(constant-analyzer
  ConstantAnalyzer.cpp
  )

target_link_libraries(constant-analyzer
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )
