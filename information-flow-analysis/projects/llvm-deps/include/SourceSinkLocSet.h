#ifndef SOURCESINKLOCSET_H
#define SOURCESINKLOCSET_H

#include <set>
#include <PointsToInterface.h>

using namespace llvm;

namespace deps{

typedef std::set<const AbstractLoc *>::iterator LocSetIter;

class SourceSinkLocSet {
 private:
  std::set<const AbstractLoc *> sourceLocs;
  std::set<const AbstractLoc *> sinkSourceLocs;
 public:
  SourceSinkLocSet(){};
  std::set<const AbstractLoc *> getSourceLocs() {
    return sourceLocs;
  }
  std::set<const AbstractLoc *> getSinkSourceLocs() {
    return sinkSourceLocs;
  }
  void insertSource(LocSetIter begin, LocSetIter end);
  void insertSinkSource(LocSetIter begin, LocSetIter end);
};
}



#endif
