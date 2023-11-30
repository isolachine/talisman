//===-- MTSolve.cpp ---------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Solve for greatest and least using two threads.
//
//===----------------------------------------------------------------------===//

#include "Constraints/PartialSolution.h"
#include "Constraints/RLConstraintKit.h"
#include "Constraints/RLConstraints.h"
#include "Constraints/SolverThread.h"
#include "Infoflow.h"

#include "llvm/Support/Atomic.h"
#include "llvm/Support/Threading.h"
#include <cassert>
#include <pthread.h>

using namespace llvm;

namespace deps {

SolverThread *SolverThread::spawn(Constraints &C, bool greatest) {
  SolverThread *T = new SolverThread(C, greatest);

  if (::pthread_create(&T->thread, NULL, solve, T)) {
    assert(0 && "Failed to create thread?!");
  }

  return T;
}

void *SolverThread::solve(void *arg) {
  assert(arg);
  SolverThread *T = (SolverThread *)arg;
  PartialSolution *P = new PartialSolution(T->C, T->greatest);
  return (void *)P;
}

void SolverThread::join(PartialSolution *&P) {
  ::pthread_join(thread, (void **)&P);
}

void RLConstraintKit::solveMT(std::string kind, Predicate *pred) {
  assert(lockedConstraintKinds[pred].insert(kind).second && "Already solved");
  assert(!leastSolutions[pred].count(kind));
  assert(!greatestSolutions[pred].count(kind));

  PartialSolution *&G = greatestSolutions[pred][kind];
  PartialSolution *&L = leastSolutions[pred][kind];

  Constraints &C = getOrCreateConstraintSet(kind, *pred);

  SolverThread *TG = SolverThread::spawn(C, true);
  SolverThread *TL = SolverThread::spawn(C, false);

  TG->join(G);
  TL->join(L);

  delete TG;
  delete TL;

  assert(leastSolutions[pred].count(kind));
  assert(greatestSolutions[pred].count(kind));

  // Cleanup
  freeUnneededConstraints(kind, *pred);
}

struct MergeInfo {
  PartialSolution *Default;
  PartialSolution *DefaultSinks;
  bool useDefaultSinks;
  std::vector<PartialSolution *> Mergees;
};

void *merge(void *arg) {
  MergeInfo *MI = (MergeInfo *)arg;
  for (std::vector<PartialSolution *>::iterator I = MI->Mergees.begin(),
                                                E = MI->Mergees.end();
       I != E; ++I) {
    (*I)->mergeIn(*MI->Default);
    if (MI->useDefaultSinks) {
      (*I)->mergeIn(*MI->DefaultSinks);
    }
  }
  return NULL;
}

std::vector<PartialSolution *>
RLConstraintKit::solveLeastMT(std::vector<std::string> kinds,
                              bool useDefaultSinks, const Predicate *pred) {
  assert(leastSolutions[pred].count("default"));

  PartialSolution *P = leastSolutions[pred]["default"];
  PartialSolution *DS = leastSolutions[pred]["default-sinks"];
  std::vector<PartialSolution *> ToMerge;
  for (std::vector<std::string>::iterator kind = kinds.begin(),
                                          end = kinds.end();
       kind != end; ++kind) {
    assert(lockedConstraintKinds[pred].insert(*kind).second &&
           "Already solved");
    assert(!leastSolutions[pred].count(*kind));
    leastSolutions[pred][*kind] =
        new PartialSolution(getOrCreateConstraintSet(*kind, *pred), false);
    ToMerge.push_back(new PartialSolution(*leastSolutions[pred][*kind]));
  }

  // Make copy of the set of solutions for returning when we're done
  std::vector<PartialSolution *> Merged(ToMerge);

  const unsigned T = 16;
  // Now kick off up to 'T' jobs, each with a vector of merges to do
  MergeInfo MI[T];

  // Make sure all threads know the default solution:
  for (unsigned i = 0; i < T; ++i) {
    MI[i].Default = P;
    MI[i].DefaultSinks = DS;
    MI[i].useDefaultSinks = useDefaultSinks;
  }

  // And round-robin hand out merge jobs:
  unsigned TID = 0;
  while (!ToMerge.empty()) {
    MergeInfo *M = &MI[TID];
    TID = (TID + 1) % T;

    M->Mergees.push_back(ToMerge.back());
    ToMerge.pop_back();
  }

  // Finally, kick off all the threads
  // with non-empty work queues
  std::vector<pthread_t> Threads;
  for (unsigned i = 0; i < T; ++i) {
    MergeInfo *M = &MI[i];
    if (!M->Mergees.empty()) {
      pthread_t Thread;
      if (::pthread_create(&Thread, NULL, merge, M)) {
        assert(0 && "Failed to create thread!");
      }
      Threads.push_back(Thread);
    }
  }

  // Wait for them all to finish
  while (!Threads.empty()) {
    ::pthread_join(Threads.back(), NULL);
    Threads.pop_back();
  }

  return Merged;
}

// typedef DenseMap<const Value *, const ConsElem *> ValueConsElemMap;
// DenseMap<const AbstractLoc *, std::map<unsigned, const ConsElem *>>
//     Infoflow::locConstraintMap;
// ValueConsElemMap Infoflow::summarySinkValueConstraintMap;
// DenseMap<const Function *, const ConsElem *> Infoflow::summarySinkVargConstraintMap;

std::vector<InfoflowSolution *>
Infoflow::solveLeastMT(std::vector<std::string> kinds, bool useDefaultSinks,
                       const Predicate *pred) {
  std::vector<PartialSolution *> PS =
      kit->solveLeastMT(kinds, useDefaultSinks, pred);

  std::vector<InfoflowSolution *> Solns;
  // for (std::vector<PartialSolution *>::iterator I = PS.begin(), E = PS.end();
  //      I != E; ++I) {
  //   Solns.push_back(
  //       new InfoflowSolution(*this, *I, kit->topConstant(), kit->botConstant(),
  //                            false, /* default to untainted */
  //                            summarySinkValueConstraintMap, locConstraintMap,
  //                            summarySinkVargConstraintMap));
  // }

  return Solns;
}

} // end namespace deps
