//===- FlowRecord.h ---------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines FlowRecords. A FlowRecord relates information flow
// sources to sinks and is a relatively concise way of encoding constraints
// (e.g. from a signature).
//
// There are four types of sources/sinks:
//  - Values: An actual llvm value
//  - DirectPtr: A memory location directly pointed to by a pointer.
//               Represented with the llvm pointer value itself.
//  - ReachPtr: All of the memory locations reachable by a pointer.
//              Represented with the llvm pointer value itself.
//  - Varg: The vararg list of a function
//===----------------------------------------------------------------------===//

#ifndef FLOWRECORD_H
#define FLOWRECORD_H

#include "CallContext.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <set>

namespace deps {

using namespace llvm;

/// A FlowRecord relates information flow sources to sinks. There are three
/// types of sources/sinks:
///  - Values: An actual llvm value
///  - DirectPtr: A memory location directly pointed to by a pointer.
///               Represented with the llvm pointer value itself.
///  - ReachPtr: All of the memory locations reachable by a pointer.
///              Represented with the llvm pointer value itself.
///  - Varg: ...
class FlowRecord {
public:
  typedef SmallPtrSet<const Value *, 1> value_set;
  typedef value_set::const_iterator value_iterator;
  typedef SmallPtrSet<const Function *, 1> fun_set;
  typedef fun_set::const_iterator fun_iterator;

  unsigned long flowRecordID;
  // DefaultID = 0. from CallContext.h
  FlowRecord() : implicit(false), sourceCtxt(DefaultID), sinkCtxt(DefaultID) {}
  FlowRecord(bool type)
      : implicit(type), sourceCtxt(DefaultID), sinkCtxt(DefaultID) {}
  FlowRecord(const ContextID source, const ContextID sink)
      : implicit(false), sourceCtxt(source), sinkCtxt(sink) {}
  FlowRecord(bool type, const ContextID source, const ContextID sink)
      : implicit(type), sourceCtxt(source), sinkCtxt(sink) {}
  FlowRecord(bool type, const ContextID source, const ContextID sink, unsigned long id)
      : implicit(type), sourceCtxt(source), sinkCtxt(sink), flowRecordID(id) {}

  bool isImplicit() const { return implicit; }

  ContextID sourceContext() const { return sourceCtxt; }
  ContextID sinkContext() const { return sinkCtxt; }

  void addSourceValue(const Value &V) { valueSources.insert(&V); }
  void addSourceDirectPtr(const Value &V) { directPtrSources.insert(&V); }
  void addSourceReachablePtr(const Value &V) { reachPtrSources.insert(&V); }

  void addSinkValue(const Value &V) { valueSinks.insert(&V); }
  void addSinkDirectPtr(const Value &V) { directPtrSinks.insert(&V); }
  void addSinkReachablePtr(const Value &V) { reachPtrSinks.insert(&V); }

  template <typename it> void addSourceValue(it begin, it end) {
    valueSources.insert(begin, end);
  }
  template <typename it> void addSourceDirectPtr(it begin, it end) {
    directPtrSources.insert(begin, end);
  }
  template <typename it> void addSourceReachablePtr(it begin, it end) {
    reachPtrSources.insert(begin, end);
  }

  template <typename it> void addSinkValue(it begin, it end) {
    valueSinks.insert(begin, end);
  }
  template <typename it> void addSinkDirectPtr(it begin, it end) {
    directPtrSinks.insert(begin, end);
  }
  template <typename it> void addSinkReachablePtr(it begin, it end) {
    reachPtrSinks.insert(begin, end);
  }

  void addSourceVarg(const Function &F) { vargSources.insert(&F); }
  void addSinkVarg(const Function &F) { vargSinks.insert(&F); }
  template <typename it> void addSourceVarg(it begin, it end) {
    vargSources.insert(begin, end);
  }
  template <typename it> void addSinkVarg(it begin, it end) {
    vargSinks.insert(begin, end);
  }

  bool valueIsSink(const Value &V) const { return valueSinks.count(&V); }
  bool vargIsSink(const Function &F) const { return vargSinks.count(&F); }
  bool directPtrIsSink(const Value &V) const {
    return directPtrSinks.count(&V);
  }
  bool reachPtrIsSink(const Value &V) const { return reachPtrSinks.count(&V); }

  value_iterator source_value_begin() const { return valueSources.begin(); }
  value_iterator source_value_end() const { return valueSources.end(); }
  value_iterator source_directptr_begin() const {
    return directPtrSources.begin();
  }
  value_iterator source_directptr_end() const { return directPtrSources.end(); }
  value_iterator source_reachptr_begin() const {
    return reachPtrSources.begin();
  }
  value_iterator source_reachptr_end() const { return reachPtrSources.end(); }
  fun_iterator source_varg_begin() const { return vargSources.begin(); }
  fun_iterator source_varg_end() const { return vargSources.end(); }

  value_iterator sink_value_begin() const { return valueSinks.begin(); }
  value_iterator sink_value_end() const { return valueSinks.end(); }
  value_iterator sink_directptr_begin() const { return directPtrSinks.begin(); }
  value_iterator sink_directptr_end() const { return directPtrSinks.end(); }
  value_iterator sink_reachptr_begin() const { return reachPtrSinks.begin(); }
  value_iterator sink_reachptr_end() const { return reachPtrSinks.end(); }
  fun_iterator sink_varg_begin() const { return vargSinks.begin(); }
  fun_iterator sink_varg_end() const { return vargSinks.end(); }

  bool one_to_one_directptr() const {
    return directPtrSources.size() == 1 && directPtrSinks.size() == 1 &&
           valueSources.size() == 0 && valueSinks.size() == 0 &&
           vargSources.size() == 0 && vargSinks.size() == 0 &&
           reachPtrSources.size() == 0 && reachPtrSinks.size() == 0;
  }

  void dump() {
    errs() << "++++ Dumping Flow Record ++++ [" << this << "]\n";
    errs() << "\tSource context: " << sourceCtxt << "\n";
    errs() << "\tSink context: " << sinkCtxt << "\n";
    errs() << "\tImplicit flow: " << (implicit ? "Yes" : "No") << "\n";
    errs() << "\tID: " << flowRecordID << "\n";
    print(valueSources, "Value Sources");
    print(directPtrSources, "D Sources");
    print(reachPtrSources, "R Sources");
    print(valueSinks, "Value Sinks");
    print(directPtrSinks, "D Sinks");
    print(reachPtrSinks, "R Sinks");
    print(vargSources, "Varg Sources");
    print(vargSinks, "Varg Sinks");
    errs() << "++++ end ++++\n\n";
  }

  bool operator==(const FlowRecord &that) {
    if (this->implicit == that.implicit &&
        this->sourceCtxt == that.sourceCtxt &&
        this->sinkCtxt == that.sinkCtxt && 
        this->valueSources.size() == that.valueSources.size() && 
        this->directPtrSources.size() == that.directPtrSources.size() && 
        this->reachPtrSources.size() == that.reachPtrSources.size() && 
        this->valueSinks.size() == that.valueSinks.size() && 
        this->directPtrSinks.size() == that.directPtrSinks.size() && 
        this->reachPtrSinks.size() == that.reachPtrSinks.size() &&
        valueSetMatch(this->valueSources, that.valueSources) &&
        valueSetMatch(this->directPtrSources, that.directPtrSources) &&
        valueSetMatch(this->reachPtrSources, that.reachPtrSources) &&
        valueSetMatch(this->valueSinks, that.valueSinks) &&
        valueSetMatch(this->directPtrSinks, that.directPtrSinks) &&
        valueSetMatch(this->reachPtrSinks, that.reachPtrSinks) &&
        funcSetMatch(this->vargSources, that.vargSources) &&
        funcSetMatch(this->vargSinks, that.vargSinks)) {
      return true;
    }
    return false;
  }

private:
  bool implicit;
  value_set valueSources, directPtrSources, reachPtrSources;
  value_set valueSinks, directPtrSinks, reachPtrSinks;
  fun_set vargSources, vargSinks;
  ContextID sourceCtxt;
  ContextID sinkCtxt;
  void print(value_set vSet, std::string name) {
    if (vSet.size()) {
      errs() << "\t" << name << "\n";
      for (auto v : vSet) {
        errs() << "\t  ";
        if (isa<BasicBlock>(v))
          errs() << "BB: " << v->getName() << "\n";
        else if (isa<Function>(v))
          errs() << "Function: " << v->getName() << "\n";
        else
          v->dump();
      }
    }
  }
  void print(fun_set fSet, std::string name) {
    if (fSet.size()) {
      errs() << name << "\n";
      fun_iterator i = fSet.begin();
      fun_iterator e = fSet.end();
      for (; i != e; i++) {
        errs() << "Function " << (*i)->getName() << "\n";
      }
    }
  }
  bool valueSetMatch(const value_set &thisSet, const value_set &thatSet) {
    for (auto value = thisSet.begin(); value != thisSet.end(); value++) {
      bool match = true;
      for (auto thatValue = thatSet.begin(); thatValue != thatSet.end(); thatValue++) {
        if ((*value) == (*thatValue)) {
          match = true;
          break;
        }
        match = false;
      }
      if (!match) {
        return false;
      }
    }
    return true;
  }

  bool funcSetMatch(const fun_set &thisSet, const fun_set &thatSet) {
    for (auto func = thisSet.begin(); func != thisSet.end(); func++) {
      bool match = true;
      for (auto thatFunc = thatSet.begin(); thatFunc != thatSet.end(); thatFunc++) {
        if ((*func) == (*thatFunc)) {
          match = true;
          break;
        }
        match = false;
      }
      if (!match) {
        return false;
      }
    }
    return true;
  }
};
// typedef uintptr_t ContextID;
} // namespace deps

#endif /* FLOWRECORD_H */
