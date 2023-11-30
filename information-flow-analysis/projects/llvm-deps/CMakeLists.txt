cmake_minimum_required(VERSION 3.3)
project(llvm_deps)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES
    include/Constraints/ConstraintKit.h
    include/Constraints/DepsTypes.h
    include/Constraints/RLConsSoln.h
    include/Constraints/RLConstraint.h
    include/Constraints/RLConstraintKit.h
    include/Constraints/RLConstraints.h
    include/Constraints/PartialSolution.h
    include/Constraints/SolverThread.h
    include/CallContext.h
    include/CallSensitiveAnalysisPass.h
    include/FlowRecord.h
    include/FPCache.h
    include/Infoflow.h
    include/InfoflowSignature.h
    include/InterProcAnalysisPass.h
    include/PointsToInterface.h
    include/SignatureLibrary.h
    include/Slice.h
    include/SourceSinkAnalysis.h
    lib/Constraints/RLConsSoln.cpp
    lib/Constraints/RLConstraintKit.cpp
    lib/Constraints/RLConstraints.cpp
    lib/Constraints/MTSolve.cpp
    lib/Constraints/PartialSolution.cpp
    lib/Constraints/Test.cpp
    lib/Deps/CallContext.cpp
    lib/Deps/Infoflow.cpp
    lib/Deps/InfoflowSignature.cpp
    lib/Deps/SignatureLibrary.cpp
    lib/Deps/Slice.cpp
    lib/Deps/StdLibSignatures.cpp
    lib/Deps/StdLibSignatures.h
    lib/PointsToInterface/PointsToInterface.cpp
    lib/SourceSinkAnalysis/SourceSinkAnalysis.cpp)

add_executable(llvm_deps ${SOURCE_FILES})