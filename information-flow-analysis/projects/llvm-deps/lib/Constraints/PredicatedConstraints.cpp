#include "Constraints/PredicatedConstraints.h"
#include "Constraints/Intervals.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#define DEFAULT_INT_RETVAL INT_MAX
#define DEFAULT_UNSIGNEDLONG_RETVAL LONG_MAX

namespace deps {
Predicate *Predicate::trueSingleton = NULL;

Predicate::Predicate() { pred = NULL; }

Predicate::Predicate(long L1, long U1, char var1) {
  pred = new Interval(L1, U1, var1);
}

Predicate::Predicate(long L1, long U1, char var1, long L2, long U2, char var2) {
  pred = new Interval(L1, U1, var1);
  addinequality(L2, U2, var2);
}

Predicate::Predicate(long L1, long U1, char var1, long L2, long U2, char var2,
                     long L3, long U3, char var3) {
  pred = new Interval(L1, U1, var1);
  addinequality(L2, U2, var2);
  addinequality(L3, U3, var3);
}

Predicate::~Predicate() {
  if (pred) {
    for (auto I : pred->intervals) {
      if (I) {
        delete I;
      }
    }
  }
  llvm::errs() << "Destructor runs\n";
}

const Predicate &Predicate::TruePred() {
  if (Predicate::trueSingleton == NULL) {
    Predicate::trueSingleton = new Predicate();
  }
  return *Predicate::trueSingleton;
}

void Predicate::addinequality(long L, long U, char var) {
  /*Checks if pred is not NULL. If pred is not NULL, then intervals
    cannot be NULL as only way pred is initialized is when a 1Dinterval
    is pushed into it.*/
  if (!pred)
    pred = new Interval(L, U, var);
  else
    pred->intervals.push_back(new Interval_1D(L, U, var));

  /*If there are multiple 1Dintervals with same var-name in the predicate
    then find the intersection of all those 1D intervals, and store a single
    1DInterval instead of the multiple 1Dintervals. */
  for (auto i = pred->intervals.begin(); i != pred->intervals.end(); ++i) {
    long L1 = (*i)->L;
    long U1 = (*i)->U;
    char v1 = (*i)->var;
    bool empty = false;
    /*In innerloop, j is incremented only when the 1DInterval given by *i
      doesn't overlap with 1DInterval given by *j, because if overlap happens
      then 1DInterval given by *j is deleted. */
    for (auto j = i + 1; j != pred->intervals.end();) {
      long L2 = (*j)->L;
      long U2 = (*j)->U;
      char v2 = (*j)->var;
      if (v1 == v2) {
        bool overlap = (L1 <= L2 && U1 >= L2) || (L2 <= L1 && U2 >= L1);
        if (overlap) {
          (*i)->L = std::max(L1, L2);
          (*i)->U = std::min(U1, U2);
          pred->intervals.erase(j);
          continue;
        } // This also takes care of the case [L1,U1] = [L2,U2]
        else {
          (*i)->L = P_NEGINF;
          (*j)->U = P_NEGINF;
          empty = true;
          break;
        } // In this case, the interval should be empty
      }
      ++j;
    }
    if (empty)
      break;
  }
  // Validate the constraints
  validate();
  // Sort 1Dinterval by variable name
  std::sort(pred->intervals.begin(), pred->intervals.end(), intervalcompare);
}

void Predicate::dump() const {
  llvm::errs() << "iter\n";
  llvm::errs() << pred << "\n";
  if (!pred) {
    llvm::errs() << "(True): \n";
    return;
  }

  for (unsigned long i = 0; i < pred->intervals.size(); i++) {
    llvm::errs() << pred->intervals[i]->L << " <= " << pred->intervals[i]->var
                 << " <= " << pred->intervals[i]->U;
    if (i < pred->intervals.size() - 1)
      llvm::errs() << ", ";
    else
      llvm::errs() << ": ";
  }
}

bool Predicate::isOverlapping(Predicate *P, int *flag) {
  bool overlap = true;
  std::vector<int> flags; /* Stores a flagvalue of -1,0,1 or 2
                             for each common variable's 1D interval
                             of current predicate and P. Flagvalue =
                             -1/0/1/2 iff current predicate's 1DInterval
                             is contained in/non-subset intersection
                             /contains/equal to P's 1D interval of same
                             variable respectively. */
  unsigned long i, j, f;  // Loop indices
  unsigned long const Isize = this->pred->intervals.size();
  unsigned long const Jsize = P->pred->intervals.size();
  int flagtemp;                        // Is a temp variable for flag
  if (this->isEmpty() || P->isEmpty()) // Checks if Predicates are empty
    return false;
  for (i = 0, j = 0; (i < Isize) || (j < Jsize);) {
    /* If i >= Isize, then P has more variables than current predicate.
       If v is one such variable, then P's 1DInterval for v is contained in
       current predicate's 1DInterval for v (latter is [NEGINF, INF]).
       Similarly if j >= Jsize. */
    if (i >= Isize) {
      flags.push_back(1);
      break;
    }
    if (j >= Jsize) {
      flags.push_back(-1);
      break;
    }
    long L1 = this->pred->intervals[i]->L;
    long U1 = this->pred->intervals[i]->U;
    char v1 = this->pred->intervals[i]->var;
    long L2 = P->pred->intervals[j]->L;
    long U2 = P->pred->intervals[j]->U;
    char v2 = P->pred->intervals[j]->var;
    /* If v1 < v2, then P cannot contain v1 as variables are stored in
       sorted order in P. And P's 1DInterval for v1 must contain
       current predicate's 1DInterval for v1. Similarly if v1 > v2.
       Execute rest of code in loop only if v1 == v2.*/
    if (v1 < v2) {
      flags.push_back(-1);
      i++;
      continue;
    } else if (v1 > v2) {
      flags.push_back(1);
      j++;
      continue;
    } else {
      /*overlap1D = true iff current predicate's 1DInterval for v overlaps
        with P's 1DInterval for v, where v = v1 = v2. And overlap = true
        iff overlap1D = true for all common variables v of current predicate
        and P. For non-common variables, overlap1D is always true.*/
      bool overlap1D = (L1 <= L2 && U1 >= L2) || (L2 <= L1 && U2 >= L1);
      overlap = overlap && overlap1D;
      if (!overlap)
        break;
      /* Let v = v1 = v2. Then push 1/-1/0 if current predicate's 1DInterval
          for v is contains/is contained in/ has non-subset intersection with
          P's 1DInterval for v respectively.*/
      if (L1 == L2 && U2 < U1)
        flags.push_back(1);
      else if (L1 == L2 && U1 < U2)
        flags.push_back(-1);
      else if (L1 == L2 && U1 == U2)
        flags.push_back(2);
      else if (L1 < L2 && U2 <= U1)
        flags.push_back(1);
      else if (L2 < L1 && U1 <= U2)
        flags.push_back(-1);
      else
        flags.push_back(0);
    }
    i++, j++;
  }
  if (!overlap) {
    *flag = DEFAULT_INT_RETVAL;
    return overlap;
  } // Case for non-overlapping predicates
  for (f = 0; f < flags.size(); ++f)
    if (flags[f] != 2)
      break;
  if (f == flags.size()) {
    *flag = 1;
    return overlap;
  } // Case for equal predicates
  /* For overlapping non-equal predicates, current predicate is contained
     in P iff every element in flags is 1; P is contained in current
     predicate iff every element in flags is -1; and for any other case,
     current predicate and P have non-subset overlap. */
  flagtemp = flags[f]; // Store 1st value not equalto 2 from flags in flagtemp
  for (unsigned long f2 = (f + 1); f2 < flags.size(); f2++) {
    if (flags[f2] == 2)
      continue;
    if (flagtemp != flags[f2]) {
      flagtemp = 0;
      break;
    }
  }
  *flag = flagtemp;
  return overlap;
}

std::vector<Predicate *> *Predicate::partitionPredicatePair(Predicate *P,
                                                            int flag) {
  static std::vector<Predicate *> retval[3]; /*retval[0] stores (this - P),
    retval[1] stores (P - this) and retval[2] stores intersection(this,P)*/
  Predicate *predIntersect = NULL;
  switch (flag) {
  case 1: // P is a subset of this
    retval[0] = partitionsubsetPredicatePair(this, P);
    this->makeEmpty();
    break;
  case -1: // this is a subset of P
    retval[1] = partitionsubsetPredicatePair(P, this);
    P->makeEmpty();
    break;
  case 0: // No Subset intersection
    predIntersect = this->intersection(P);
    retval[0] = partitionsubsetPredicatePair(this, predIntersect);
    retval[1] = partitionsubsetPredicatePair(P, predIntersect);
    retval[2].push_back(predIntersect);
    this->makeEmpty();
    P->makeEmpty();
    break;
  default:
    break;
  }
  return retval;
}

std::vector<Predicate *>
Predicate::partitionsubsetPredicatePair(Predicate *P1, Predicate *P2) {
  std::vector<Predicate *> retval;
  Predicate *tempPred = new Predicate();
  /*P2 is assumed to be a subset of P1.
    Description of Algorithm: Let P1 and P2 be n dimensional with
    1Dintervals I1, I2, .. In and J1, J2, .. Jn respectively(if P1
    doesn't have a 1DInterval for some variable present in P2, then it's
    corresponding interval is [NEG_INF, INF]). Then iterate over the
    1DIntervals of P1. For each Ik (1 <= k <= n), define two predicates:
    P3(k) := J1, J2, .. J(k-1), left(Ik-Jk), I(k+1), I(k+2), .. In
    P4(k) := J1, J2, .. J(k-1), right(Ik-Jk), I(k+1), I(k+2), .. In
    Add P3(k) and P4(k) to retval. Then,
    retval = {P3(k), P4(k)}_{1 <= k <= n} = P1 - P2.
    The inequalities J1, J2, .. J(k-1) are stored in tempPred for
    the k^th iteration.
    As P2 is assumed to be a subset of P1, hence var(P1) is a subset of
    var(P2). Hence we just iterate over the variables in P2.*/
  for (unsigned long j = 0; j < P2->pred->intervals.size(); j++) {
    char v2 = P2->pred->intervals[j]->var;
    long L2 = P2->pred->intervals[j]->L;
    long U2 = P2->pred->intervals[j]->U;
    /*Check if v2 is present in P1. If so, get lower and upper bound
      values for 1DInterval of v2 in P1.*/
    unsigned long i = P1->char_to_index(v2);
    long L1, U1;
    if (i != DEFAULT_UNSIGNEDLONG_RETVAL) {
      L1 = P1->pred->intervals[i]->L;
      U1 = P1->pred->intervals[i]->U;
    } // Variable found in P1
    else {
      L1 = P_NEGINF;
      U1 = P_INF;
    } // Variable not found in P1
    // Define P3(k) and P4(k) as mentioned in description of algorithm
    Predicate *P3 = new Predicate(L1, sub1_extend(L2), v2);
    Predicate *P4 = new Predicate(add1_extend(U2), U1, v2);
    P3->validate();
    P4->validate();
    /* Copy inequalities of P1 and tempPred in (both) P3(k) and P4(k). As P2
       is a subset of P1, intersection(Ik, Jk) = Jk forall k. Hence to
       simplify the code, instead of copying only I(k+1), ... I(n) into
       P3(k) and P4(k), we copy all of I1, I2, ... In into P3(k) and P4(k).
       The 1D interval's for same variable are handled in the addinequality
       function which stores only the intersection of all 1DIntervals with
       same variable name. */
    for (unsigned long k = 0; k < P1->pred->intervals.size(); k++) {
      char var = P1->pred->intervals[k]->var;
      unsigned long L = P1->pred->intervals[k]->L;
      unsigned long U = P1->pred->intervals[k]->U;
      P3->addinequality(L, U, var);
      P4->addinequality(L, U, var);
    }
    if (tempPred->pred) {
      for (unsigned long k = 0; k < tempPred->pred->intervals.size(); k++) {
        char var = tempPred->pred->intervals[k]->var;
        unsigned long L = tempPred->pred->intervals[k]->L;
        unsigned long U = tempPred->pred->intervals[k]->U;
        P3->addinequality(L, U, var);
        P4->addinequality(L, U, var);
      }
    }
    // Update retval
    retval.push_back(P3);
    retval.push_back(P4);
    // Update inequality in tempPred
    tempPred->addinequality(L2, U2, v2);
  }
  // Remove empty predicates from retval
  for (unsigned long i = 0; i < retval.size(); i++) {
    if (retval[i]->isEmpty()) {
      retval.erase(retval.begin() + i);
      i--;
    }
  }
  return retval;
}

Predicate *Predicate::intersection(Predicate *P) {
  Predicate *predIntersect = new Predicate();
  unsigned long const Isize = this->pred->intervals.size();
  unsigned long const Jsize = P->pred->intervals.size();
  /* Loop iterates over set of all variables in union(var(this), var(P))
     from least variable (value) to largest variable (value). */
  for (unsigned long i = 0, j = 0; (i < Isize) || (j < Jsize);) {
    /*If i >= Isize but j < Jsize, then there are variables in P
      which are not in current predicate. If v is any such variable,
      then current predicate's 1DInterval for v is assumed as [NEGINF, INF].
      Thus the intersection is equalto P's 1DInterval for v, so copy
      P's 1DInterval for v into predIntersect. Similarly if j >= Jsize. */
    if (i >= Isize) {
      long L2 = P->pred->intervals[j]->L;
      long U2 = P->pred->intervals[j]->U;
      char v2 = P->pred->intervals[j]->var;
      predIntersect->addinequality(L2, U2, v2);
      j++;
      continue;
    }
    if (j >= Jsize) {
      long L1 = this->pred->intervals[i]->L;
      long U1 = this->pred->intervals[i]->U;
      char v1 = this->pred->intervals[i]->var;
      predIntersect->addinequality(L1, U1, v1);
      i++;
      continue;
    }
    long L1 = this->pred->intervals[i]->L;
    long U1 = this->pred->intervals[i]->U;
    char v1 = this->pred->intervals[i]->var;
    long L2 = P->pred->intervals[j]->L;
    long U2 = P->pred->intervals[j]->U;
    char v2 = P->pred->intervals[j]->var;
    /*If v1 < v2, then v1 does not have a 1DInterval in P as
      variables are stored in sorted order. Hence P's 1DInteral
      for v1 is assumed to be [NEGINF, INF]. Thus intersection
      is equalto current predicate's 1DInterval for v1. Only increment i,
      as current predicate may have a 1DInterval for v2. Similarly if
      v1 > v2. If v1 = v2, find intersection of 1Dintervals, and store
      in predIntersect.*/
    if (v1 < v2) {
      predIntersect->addinequality(L1, U1, v1);
      i++;
      continue;
    } else if (v1 > v2) {
      predIntersect->addinequality(L2, U2, v2);
      j++;
      continue;
    } else {
      predIntersect->addinequality(std::max(L1, L2), std::min(U1, U2), v1);
    }
    i++, j++;
  }
  predIntersect->validate();
  return predIntersect;
}

bool Predicate::isEmpty() {
  bool empty = false;
  /* If any 1DInterval of Predicate is empty, then predicate is empty. */
  for (unsigned long i = 0; i < pred->intervals.size(); i++) {
    bool empty1D = (pred->intervals[i]->L == P_NEGINF) &&
                   (pred->intervals[i]->U == P_NEGINF);
    empty = empty || empty1D;
  }
  return empty;
}

/*Each 1Dinterval of the predicate is set to [NEGINF, NEGINF] i.e. the
  default empty 1Dinterval. */
void Predicate::makeEmpty() {
  for (unsigned long i = 0; i < pred->intervals.size(); ++i) {
    pred->intervals[i]->L = P_NEGINF;
    pred->intervals[i]->U = P_NEGINF;
  }
}

void Predicate::validate() {
  bool invalid = false;
  /*Predicate is invalid/empty if for ANY of it's 1Dinterval, either
    it's lower bound (L) is greater than it's upper bound (U),
    or L = U = INF or L = U = NEG_INF */
  for (unsigned long i = 0; i < pred->intervals.size(); ++i) {
    bool invalid1, invalid2, invalid3;
    long L = pred->intervals[i]->L;
    long U = pred->intervals[i]->U;
    invalid1 = (L > U);
    invalid2 = (L == P_INF) && (U == P_INF);
    invalid3 = (L == P_NEGINF) && (U == P_NEGINF);
    invalid = (invalid1 || invalid2 || invalid3);
    if (invalid)
      break;
  }
  /*Make invalid predicate empty*/
  if (invalid)
    makeEmpty();
  return;
}

long Predicate::add1_extend(long x) { return (x == P_INF) ? P_INF : (x + 1); }

long Predicate::sub1_extend(long x) {
  return (x == P_NEGINF) ? P_NEGINF : (x - 1);
}

bool Predicate::predcompare(Predicate *P1, Predicate *P2) {
  unsigned long size =
      std::min(P1->pred->intervals.size(), P2->pred->intervals.size());
  /* P1 and P2's intervals have been stored in sorted order wrt their
     variable names. P1 <= P2 if
     (i) P1's smallest variable < P2's smallest variable
     (ii) If P1's smallest variable == P2's smallest
          variable == v, the P1's lower bound for
          v is < P2's lower bound for v.
     (iii) If lower bounds are equal, then move onto next
           smallest variable, and repeat (i) and (ii).
           If P2 runs out of variables, then P1 <= P2.
           Similarly if P1 runs out of variables.
     (iv)  If (iii) doesn't hold, then P1 must be equalto P2.
           In this case, we return true.
  */
  for (unsigned long i = 0; i < size; i++) {
    if (P1->pred->intervals[i]->var != P2->pred->intervals[i]->var)
      return (P1->pred->intervals[i]->var < P2->pred->intervals[i]->var);
    else if (P1->pred->intervals[i]->L != P2->pred->intervals[i]->L)
      return (P1->pred->intervals[i]->L < P2->pred->intervals[i]->L);
    else
      continue;
  }
  if (P1->pred->intervals.size() != P2->pred->intervals.size())
    return (P1->pred->intervals.size() < P2->pred->intervals.size());
  return true;
}

/*This is a interval-compare function for 1DIntervals of the SAME predicate
  - it compares 1D intervals based on variable value */
bool Predicate::intervalcompare(Interval_1D *I1, Interval_1D *I2) {
  return (I1->var < I2->var);
}

/*Searches for the v's 1DInterval in the predicate, if found returns position
  in intervals, else returns DEFAULT_UNSIGNEDLONG_RETVAL*/
unsigned long Predicate::char_to_index(char v) {
  for (unsigned long i = 0; i < this->pred->intervals.size(); i++) {
    if (this->pred->intervals[i]->var == v)
      return i;
  }
  return DEFAULT_UNSIGNEDLONG_RETVAL;
}

} // namespace deps