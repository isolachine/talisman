//===-- SolverThread.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// TODO Description
//
//===----------------------------------------------------------------------===//

#ifndef _SOLVERTHREAD_H_
#define _SOLVERTHREAD_H_

#include "Constraints/PartialSolution.h"
#include "Constraints/RLConstraintKit.h"
#include "Constraints/RLConstraints.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/Threading.h"

#include <cassert>

namespace deps {

typedef std::vector<RLConstraint> Constraints;

// Spawns thread to solve the given set of Constraints.
class SolverThread {
  pthread_t thread;

  Constraints &C;
  bool greatest;

  SolverThread(Constraints &C, bool isG) : C(C), greatest(isG) {}

  static void *solve(void *arg);

public:
  // Create a new thread to solve the given constraints
  static SolverThread *spawn(Constraints &C, bool greatest);

  // Wait for this thread to finish
  void join(PartialSolution *&P);

  // Please just call join() explicitly
  // But just in case...
  ~SolverThread() {
    PartialSolution *P;
    join(P);
  }
};

} // end namespace deps

#endif // _SOLVERTHREAD_H_
