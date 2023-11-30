#include "SourceSinkLocSet.h"

namespace deps {
void SourceSinkLocSet::insertSource(LocSetIter begin, LocSetIter end) {
  sourceLocs.insert(begin, end);
}

void SourceSinkLocSet::insertSinkSource(LocSetIter begin, LocSetIter end) {
  sinkSourceLocs.insert(begin, end);
}
}
