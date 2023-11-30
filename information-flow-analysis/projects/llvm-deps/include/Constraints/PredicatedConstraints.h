#ifndef PREDCONSTRAINTS_H_
#define PREDCONSTRAINTS_H_

#include "Constraints/Intervals.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <vector>

namespace deps {

class Predicate {

protected:
  static Predicate *trueSingleton;

public:
  /*Constructors: */
  Predicate();
  Predicate(long L1, long U1, char var1);
  Predicate(long L1, long U1, char var1, long L2, long U2, char var2);
  Predicate(long L1, long U1, char var1, long L2, long U2, char var2, long L3,
            long U3, char var3);

  /*Destructor */
  ~Predicate();

  static const Predicate &TruePred();
  Interval *pred;

  /**************************************************************************
  Function Name : addinequality
  Arguments     : L   - lower bound of inequality
                  U   - upper bound of inequality
                  var - variable name for inequality
  Description   : Adds another inequality (dimension) to the predicate's
                  intervals
  Return Value  : -
  **************************************************************************/
  void addinequality(long L, long U, char var);

  /**************************************************************************
  Function Name : dump
  Arguments     : -
  Description   : Prints the predicate's intervals
  Return Value  : -
  **************************************************************************/
  virtual void dump() const;

  /**************************************************************************
  Function Name : isOverlapping
  Arguments     : P    - Another predicate
                  flag - Is used as another return type for function. If there
                         is overlap, flag gives the type of overlap:
                             flag  = 1 <-> P is contained in this predicate
                             flag = -1 <-> this predicate is contained in P
                             flag = 0 <-> non subset overlap
  Description   : Checks whether this predicate overlaps with predicate P, and
                  gives type of overlap through argument flag
  Return Value  : Returns true iff P overlaps with this predicate
  **************************************************************************/
  bool isOverlapping(Predicate *P, int *flag);

  /**************************************************************************
  Function Name : partitionPredicatePair
  Arguments     : P    - Another predicate
                  flag - Gives type of overlap as given by function
                         'isOverlapping'
  Description   : Let this predicate be denoted by Q. Then depending on the
                  type of overlap between Q and P, it gives Q/P, P/Q and
                  intersection(Q,P) as a vector of intervals, and accordingly
                  makes Q or P's intervals empty.
  Return Value  : Returns an array of vectors (say A). Then,
                    A[0] - vector of intervals from (Q - P)
                    A[1] - vector of intervals from (P - Q)
                    A[2] - the intersection interval of Q and P.
  Warnings      : This function generates a warning 'redundant move in ...
                  std::move'. This is because we return an array of vectors
                  (this warning can be ignored).
  **************************************************************************/
  std::vector<Predicate *> *partitionPredicatePair(Predicate *P, int flag);

  /**************************************************************************
  Function Name : partitionsubsetPredicatePair
  Arguments     : P1 - A predicate
                  P2 - Another predicate. P2 must be contained in P1.
  Description   : It assumes that P2 is contained in P1. It returns P1/P2 as
                  a vector of intervals.
  Return Value  : Returns P1 - P2 as a vector of intervals.
  Warnings      : This function generates a warning 'redundant move in ...
                  std::move'. This is because we return a vectors.
                  (this warning can be ignored).
  **************************************************************************/
  static std::vector<Predicate *> partitionsubsetPredicatePair(Predicate *P1,
                                                               Predicate *P2);

  /**************************************************************************
  Function Name : intersection
  Arguments     : P - Another predicate
  Description   : It returns the intersection of current predicate and P
  Return Value  : intersection(current predicate, P)
  **************************************************************************/
  Predicate *intersection(Predicate *P);

  /**************************************************************************
  Function Name : isEmpty
  Arguments     : -
  Description   : Checks if current predicate's interval is empty
  Return Value  : Returns true iff current predicate is empty
  **************************************************************************/
  bool isEmpty();

  /**************************************************************************
  Function Name : makeEmpty
  Arguments     : -
  Description   : Makes the current predicate's interval empty
  Return Value  : -
  **************************************************************************/
  void makeEmpty();

  /**************************************************************************
  Function Name : validate
  Arguments     : -
  Description   : Checks if current predicate's interval is a valid interval
                  and if not makes it empty.
  Return Value  : -
  **************************************************************************/
  void validate();

  // D. REORDERING THIS IN .CPP GIVES AN ERROR!
  /**************************************************************************
  Function Name : add1_extend
  Arguments     : x - long number
  Description   : Bounded addition of 1
  Return Value  : min(x+1, P_INF)
  **************************************************************************/
  static long add1_extend(long x);

  /**************************************************************************
  Function Name : sub1_extend
  Arguments     : x
  Description   : Bounded subtraction of 1
  Return Value  : max(x-1, P_NEGINF)
  **************************************************************************/
  static long sub1_extend(long x);

  /**************************************************************************
  Function Name : predcompare
  Arguments     : P1 - A predicate
                  P2 - Another predicate
  Description   : Defines a <= relation on predicates
  Return Value  : True iff P1 <= P2
  **************************************************************************/
  static bool predcompare(Predicate *P1, Predicate *P2);

  /**************************************************************************
  Function Name : intervalcompare
  Arguments     : I1 - A interval
                  I2 - Another interval
  Description   : Defines a < relation on Interval_1D
  Return Value  : True iff I1 < I2
  **************************************************************************/
  static bool intervalcompare(Interval_1D *I1, Interval_1D *I2);

  /**************************************************************************
  Function Name : char_to_index
  Arguments     : v - a variable name
  Description   : Finds the 1Dinterval corresponding to variable name v, and
                  returns it's index in the vector intervals.
  Return Value  : Index of 1Dinterval corresponding to varname v, else
                  returns DEFAULT_UNSIGNEDLONG_RETVAL
  **************************************************************************/
  unsigned long char_to_index(char v);
};

} // namespace deps

#endif