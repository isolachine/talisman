#ifndef DEBUG_TYPE
#define DEBUG_TYPE "taint"

#include "TaintAnalysisBase.h"

using namespace llvm;
namespace deps {
STATISTIC(NumSourceConstrained, "Number of sources constrained");
STATISTIC(NumSinkConstrained, "Number of sinks constrained");

const Function *findEnclosingFunc(const Value *V) {
  if (const Argument *Arg = dyn_cast<Argument>(V)) {
    return Arg->getParent();
  }
  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    return I->getParent()->getParent();
  }
  return NULL;
}

bool TaintAnalysisBase::hasPointerTarget(const AbstractLoc *loc) {
  bool linkExists = false;
  if (loc != NULL && loc->getSize() > 0)
    linkExists = loc->hasLink(0);

  if (linkExists) {
    AbstractLoc *link = loc->getLink(0).getNode();

    if (link != NULL)
      linkExists = link->getSize() > 0;
    else
      linkExists = false;
  }

  return linkExists;
}

std::map<unsigned, const ConsElem *>
TaintAnalysisBase::getPointerTarget(const AbstractLoc *loc) {
  // If the value is a pointer, use pointsto analysis to resolve the target
  const DSNodeHandle nh = loc->getLink(0);
  const AbstractLoc *node = nh.getNode();
  errs() << "Linked Node";
  if (node == NULL || node->getSize() == 0)
    return std::map<unsigned, const ConsElem *>();

  node->dump();
  DenseMap<const AbstractLoc *, std::map<unsigned, const ConsElem *>>::iterator
      childElem = ifa->locConstraintMap.find(node);
  // Instead look at this set of constraint elements
  if (childElem != ifa->locConstraintMap.end())
    return childElem->second;
  else
    return std::map<unsigned, const ConsElem *>();
}

void TaintAnalysisBase::constrainValue(std::string kind, const Value &value,
                                       int t_offset, std::string match_name,
                                       RLLabel label) {

  std::string s = value.getName();
  std::string meta = "[" + match_name + ":" + std::to_string(t_offset) +
                     "] [SrcIdx:" + std::to_string(NumSourceConstrained) + "]";
  DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG,
                  errs() << "Trying to constrain " << meta << " for value : ";
                  value.dump(););
  const AbstractLocSet &locs = ifa->locsForValue(value);
  const AbstractLocSet &rlocs = ifa->reachableLocsForValue(value);
  if (t_offset < 0 || (locs.size() == 0 && rlocs.size() == 0)) {
    DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG,
                    errs() << "SETTING " << s << " TO BE TAINTED\n";);
    ifa->constrainAllConsElem(kind, value, std::set<const ConsElem *>(), label,
                              meta);
  }

  // Heap nodes not returned from locs For value
  AbstractLocSet relevantLocs{locs.begin(), locs.end()};
  for (auto &rl : rlocs) {
    if (rl->isHeapNode()) {
      relevantLocs.insert(rl);
    }
  }
  DEBUG_WITH_TYPE(
      DEBUG_TYPE_DEBUG, errs() << "\n --------- locs --------- \n";
      for (auto &l
           : locs) {
        l->dump();
        errs() << " --------- \n";
      } errs()
      << " --------- rlocs --------- \n";
      for (auto &rl
           : rlocs) {
        rl->dump();
        errs() << " --------- \n";
      } errs()
      << " locs size : " << locs.size() << "\n";
      errs() << " rlocs size : " << rlocs.size() << "\n";
      errs() << " relevantLocs size : " << relevantLocs.size() << "\n";
      errs() << " --------- end --------- \n\n";);

  unsigned offset = 0, span = 0;
  bool hasOffset = ifa->offsetForValue(value, &offset);
  unsigned numElements = getNumElements(value);
  DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG, errs() << "This value has " << numElements
                                           << " elements.\n";);

  if (!ifa->offset_used) {
    t_offset = -1; // if offset is disabled ignore offset from taintfile
    hasOffset = false;
  }

  AbstractLocSet::const_iterator loc = relevantLocs.begin();
  AbstractLocSet::const_iterator end = relevantLocs.end();

  std::set<const ConsElem *> elementsToConstrain;
  for (; loc != end; ++loc) {
    DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG, (*loc)->dump(););
    std::map<unsigned, const ConsElem *> elemMap =
        ifa->getOrCreateConsElem(**loc);
    if (((*loc)->isNodeCompletelyFolded() ||
         (*loc)->type_begin() == (*loc)->type_end()) &&
        elemMap.size() <= 1) {
      hasOffset = false;
    } else if (t_offset >= 0) {
      fieldIndexToByteOffset(t_offset, &value, *loc, &offset, &span);
      hasOffset = true;
    }

    if (hasOffset) {
      DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG,
                      errs() << "Has offset [" << offset << "]\n";);
      std::set<const ConsElem *> rel;

#if HOTSPOT
      if (ifa->invertedLocConstraintMap.find(*loc) !=
              ifa->invertedLocConstraintMap.end() &&
          ifa->invertedLocConstraintMap[*loc].size() > 1) {
        auto VSet = ifa->invertedLocConstraintMap[*loc];

        // for (auto p : ifa->summarySourceValueConstraintMap) {
        //   errs() << "\nv - ce pair: ";
        //   p.getFirst()->dump();
        //   errs() << " === ";
        //   p.getSecond()->dump(errs());
        // }

        // errs() << " \n-----------++----------\n ";
        // for (auto p : ifa->summarySinkValueConstraintMap) {
        //   errs() << "\nv - ce pair: ";
        //   p.getFirst()->dump();
        //   errs() << " === ";
        //   p.getSecond()->dump(errs());
        // }

        if (VSet.size() > 1) {
          auto elem = ifa->getOrCreateConsElem(**loc).at(offset);
          if (auto varElem = dyn_cast<RLConsVar>(elem)) {
            for (auto v : VSet) {
              v->dump();
              bool caught = false;
              for (auto ctxt : ifa->valueConstraintMap) {
                auto sourceElem = ctxt.getSecond().find(v);
                if (sourceElem != ctxt.getSecond().end()) {
                  (*sourceElem).getSecond()->dump(errs() << "this CE:");
                  errs() << "\n";
                  ifa->kit->addConstraint(
                      ifa->kindFromImplicitSink(false, false),
                      *(*sourceElem).getSecond(), *elem,
                      " ;  [ConsDebugTag-23]");
                  if (v == &value) {
                    rel.insert((*sourceElem).getSecond());
                  }
                  caught = true;
                }
              }
              if (!caught) {
                std::string varMeta = ifa->getOriginalLocationConsElem(v);
                const ConsElem &singleElem =
                    ifa->kit->newVar(varElem->getDesc(), varMeta);
                ifa->kit->addConstraint(ifa->kindFromImplicitSink(false, false),
                                        singleElem, *elem,
                                        " ;  [ConsDebugTag-24]");
                if (v == &value) {
                  rel.insert(&singleElem);
                }
              }
            }
          }
        }
      } else {
#endif
        rel = gatherRelevantConsElems(*loc, offset, span, numElements, t_offset,
                                      value, true);
#if HOTSPOT
      }
#endif
      DEBUG_WITH_TYPE(
          DEBUG_TYPE_DEBUG, errs() << "REL: \n"; for (auto e
                                                      : rel) {
            e->dump(errs() << "");
            errs() << "\n";
          });
      elementsToConstrain.insert(rel.begin(), rel.end());
    } else {
      DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG, errs() << "No offset.\n";);
      for (auto &locs : relevantLocs) {
        DSNode::LinkMapTy edges{locs->edge_begin(), locs->edge_end()};
        for (auto &edge : edges) {
          const DSNode *n = edge.second.getNode();
          if (n != NULL) {
            auto locConstraintsMap = ifa->locConstraintMap.find(n);
            if (locConstraintsMap != ifa->locConstraintMap.end()) {
              for (auto &kv : locConstraintsMap->second) {
                elementsToConstrain.insert(kv.second);
              }
            }
          }
        }
      }
      auto locConstraintsMap = ifa->locConstraintMap.find(*loc);
      if (locConstraintsMap != ifa->locConstraintMap.end()) {
        for (auto &kv : locConstraintsMap->second) {
          elementsToConstrain.insert(kv.second);
        }
      }
    }
    DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG,
                    errs() << "FOUND " << elementsToConstrain.size()
                           << " elements from the locsForValue\n";
                    errs() << "=====\n";);
  }
  DEBUG_WITH_TYPE(
      DEBUG_TYPE_DEBUG, errs() << "Number of elements to constrain: "
                               << elementsToConstrain.size() << "\n";
      for (auto &el
           : elementsToConstrain) {
        el->dump(errs());
        errs() << " : addr " << el << "\n";
      });
  ifa->constrainAllConsElem(kind, value, elementsToConstrain, label, meta);
}

void TaintAnalysisBase::labelSink(std::string kind) {
  // process the sink functions' arguments
  DEBUG_WITH_TYPE(DEBUG_TYPE,
                  errs() << "\n========= Labeling Sink Functions ========= \n");
  for (auto var : ifa->indexedSinkVariables) {
    Value &value = *var.val;
    int t_offset = var.index;

    DEBUG_WITH_TYPE(DEBUG_TYPE, errs() << "Labeling: "
                                       << ifa->getOrCreateStringFromValue(value)
                                       << " at offset: " << t_offset << "\n";);

    std::string meta = "[" + var.name + ":" + std::to_string(t_offset) + "] [" +
                       var.callsite + ":" + std::to_string(var.number) +
                       "] [SnkIdx:" + std::to_string(NumSinkConstrained++) +
                       "]";

    const AbstractLocSet &locs = ifa->locsForValue(value);
    const AbstractLocSet &rlocs = ifa->reachableLocsForValue(value);

    // TODO: confirm the correctness of the [t_offset < 0] condition
    // It was removed for a while because we believed it was unnecessary
    if (t_offset < 0 || (locs.size() == 0 && rlocs.size() == 0)) {
      ifa->setLabel(kind, value, var.label, false, meta);
    }

    // Heap nodes not returned from locsForValue(v)
    AbstractLocSet relevantLocs{locs.begin(), locs.end()};
    for (auto &rl : rlocs) {
      if (rl->isHeapNode()) {
        relevantLocs.insert(rl);
      }
    }

    DEBUG_WITH_TYPE(
        DEBUG_TYPE_TAINT, errs() << "\n--------- locs ---------\n";
        for (auto &l
             : locs) {
          l->dump();
          errs() << "---------\n";
        };
        errs() << "--------- rlocs ---------\n"; for (auto &rl
                                                      : rlocs) {
          rl->dump();
          errs() << "---------\n";
        };
        errs() << "locs size : " << locs.size() << "\n";
        errs() << "rlocs size : " << rlocs.size() << "\n";
        errs() << "relevantLocs size : " << relevantLocs.size() << "\n";
        errs() << "--------- end ---------\n\n";);

    unsigned offset = 0, span = 0;
    bool hasOffset = ifa->offsetForValue(value, &offset);
    unsigned numElements = getNumElements(value);
    errs() << "This value has " << numElements << " elements.\n";
    errs() << "Has offset? " << (hasOffset ? "YES" : "NO")
           << ", and the offset is: " << offset << "\n";

    if (!ifa->offset_used) {
      t_offset = -1; // if offset is disabled ignore offset from taintfile
      hasOffset = false;
    }

    std::set<const ConsElem *> elementsToUntaint;
    for (auto loc = relevantLocs.begin(); loc != relevantLocs.end(); ++loc) {
      errs() << "\nUntaint node at:\n";
      (*loc)->dump();
      DSNode::TyMapTy allFields{(*loc)->type_begin(), (*loc)->type_end()};
      value.getType()->dump();

      for (auto &ty : allFields) {
        errs() << "offset: " << ty.first << " ";
        const svset<llvm::Type *> *types = ty.second;
        for (auto i = types->begin(); i != types->end(); i++) {
          (*i)->dump();
        }
      }

      if (t_offset >= 0) {
        fieldIndexToByteOffset(t_offset, &value, *loc, &offset, &span);
        errs() << "\tThe byte offset is: " << offset << "\n";
        hasOffset = true;
      } else {
        hasOffset = false;
      }

      if (hasOffset) {
        std::set<const ConsElem *> rel = gatherRelevantConsElems(
            *loc, offset, span, numElements, t_offset, value, false);
        elementsToUntaint.insert(rel.begin(), rel.end());
      } else {
        for (auto &locs : relevantLocs) {
          DSNode::LinkMapTy edges{locs->edge_begin(), locs->edge_end()};
          for (auto &edge : edges) {
            const DSNode *n = edge.second.getNode();
            if (n != NULL) {
              auto locConstraintsMap = ifa->locConstraintMap.find(n);
              if (locConstraintsMap != ifa->locConstraintMap.end()) {
                for (auto &kv : locConstraintsMap->second) {
                  elementsToUntaint.insert(kv.second);
                }
              }
            }
          }
        }
        auto offsetConsElemMap = ifa->locConstraintMap.find(*loc);
        if (offsetConsElemMap != ifa->locConstraintMap.end()) {
          for (auto &kv : offsetConsElemMap->second) {
            elementsToUntaint.insert(kv.second);
          }
        }
      }
      errs() << "FOUND " << elementsToUntaint.size()
             << " elements from the locsForValue\n";
    }

    errs() << "Number of elements to constrain: " << elementsToUntaint.size()
           << "\n";
    for (auto el : elementsToUntaint) {
      el->dump(errs());
      errs() << "\n";
    }

    if (elementsToUntaint.size() == 0) {
      ifa->setLabel(kind, value, var.label, false, meta);
    } else {
      std::string debugTag = " ;  [ConsDebugTag-*]  sink " + meta;
      const ConsElem *constant = &(ifa->kit->constant(var.label));
      for (auto e : elementsToUntaint) {
        ifa->kit->addConstraint(kind, *e, *constant, debugTag);
      }
    }
  }
}

// TODO: work with two different versions based on
// tainting source or labeling sink [bool tainting]
std::set<const ConsElem *> TaintAnalysisBase::gatherRelevantConsElems(
    const AbstractLoc *node, unsigned offset, unsigned span,
    unsigned numElements, unsigned index, const Value &val, bool tainting) {
  DenseMap<const AbstractLoc *, std::map<unsigned, const ConsElem *>>::iterator
      curElem = ifa->locConstraintMap.find(node);
  std::map<unsigned, const ConsElem *> elemMap;
  std::set<const ConsElem *> relevant;

  if (curElem == ifa->locConstraintMap.end())
    return relevant;

  elemMap = curElem->second;
  DEBUG_WITH_TYPE(
      DEBUG_TYPE_DEBUG, errs() << "\tConsElemMap: \n"; for (auto e
                                                            : elemMap) {
        errs() << "\toffset: " << e.first << ", element [";
        e.second->dump(errs());
        errs() << "] addr [" << e.second << "] added.\n";
      });
  DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG, errs() << "Map size [" << elemMap.size()
                                           << "] <--> Number of elements ["
                                           << numElements << "]\n";);
  if (elemMap.size() >= numElements) {
    if (span > 0) {
      return ifa->findRelevantConsElem(node, elemMap, offset, span);
    }
    Type *type = val.getType();
    if (PointerType *pt = dyn_cast<PointerType>(type)) {
      type = pt->getPointerElementType();
    }
    if (type->isStructTy()) {
      StructType *st = dyn_cast<StructType>(type);
      st->dump();
      errs() << "idx: " << index << "\n";
      if (index == UINT_MAX) {
        for (auto e : elemMap) {
          relevant.insert(e.second);
        }
      } else {
        st->getStructElementType(index)->dump();
        relevant = ifa->findRelevantConsElem(node, elemMap, offset,
                                             st->getStructElementType(index));
      }
    } else {
      relevant = ifa->findRelevantConsElem(node, elemMap, offset, &val);
    }
    return relevant;
  }

  // Go to other nodes if the type matches & retrieve their elements if exists
  if (hasPointerTarget(node)) {
    bool all_children = true;
    AbstractLocSet childLocs;
    Type *t = val.getType();
    if (isa<AllocaInst>(&val)) {
      t = t->getContainedType(0);
    }
    std::string tyname;
    raw_string_ostream tstr{tyname};
    tstr << *t;
    tstr.str();
    errs() << "Matching Type:" << tyname << "\n";
    if (t->isPointerTy()) {

      DSNode::TyMapTy nodetypes{node->type_begin(), node->type_end()};
      for (auto &kv : nodetypes) {
        if (node->getSize() > 0 && node->hasLink(kv.first)) {
          const AbstractLoc *child = node->getLink(kv.first).getNode();
          if (all_children) {
            childLocs.insert(child);
          } else {
            for (svset<Type *>::const_iterator ni = kv.second->begin(),
                                               ne = kv.second->end();
                 ni != ne; ++ni) {
              std::string tyname2;
              raw_string_ostream nstr{tyname2};
              nstr << **ni;
              nstr.str();
              if (tyname == tyname2) {
                errs() << "FOUND MATCHING CHILD NODE:";
                child->dump();
                childLocs.insert(child);
              }
            }
          }
        }
      }
    }

    for (auto &l : childLocs) {
      std::set<const ConsElem *> childElems = gatherRelevantConsElems(
          l, offset, span, numElements, index, val, tainting);
      relevant.insert(childElems.begin(), childElems.end());
    }
  }

  return relevant;
}

void TaintAnalysisBase::labelValue(std::string kind,
                                   std::set<ConfigVariable> vars, bool gte) {
  for (auto var : vars) {
    NumSourceConstrained++;
    if (var.type == ConfigVariableType::Constant) {
      bool useConstant = true;
      if (ifa->config.contains("using_constant"))
        useConstant = ifa->config.at("using_constant");
      if (useConstant) {
        std::string loc = var.file + ":" + std::to_string(var.line);
        auto constantConstraintMap = ifa->constantValueConstraintMap.find(loc);
        if (constantConstraintMap != ifa->constantValueConstraintMap.end()) {
          for (auto constraint : constantConstraintMap->second)
            if (constraint.first->getSExtValue() == var.value) {
              std::string meta = " ;  [ConsDebugTag-*] [Constant:" +
                                 std::to_string(var.value) + "] [SrcIdx:" +
                                 std::to_string(NumSourceConstrained) + "]";
              ifa->kit->addConstraint(kind, ifa->kit->constant(var.label),
                                      *constraint.second, meta);
              (*constraint.second).dump(errs());
            }
        }
      }
      continue;
    }
    for (DenseMap<const Value *, const ConsElem *>::const_iterator
             entry = ifa->summarySourceValueConstraintMap.begin(),
             end = ifa->summarySourceValueConstraintMap.end();
         entry != end; ++entry) {
      const Value &value = *(entry->first);

      // Only taint variables defined in taint files if the function matches
      const Function *fn = findEnclosingFunc(&value);
      bool function_matches = false;
      if (var.function.size() == 0) {
        function_matches = true;
      } else if (fn && fn->hasName() && fn->getName() == var.function) {
        function_matches = true;
      }

      if (function_matches) {
        bool variable_matches = false;
        if (value.hasName()) {
          if (value.getName() == var.name) {
            variable_matches = true;
            DEBUG_WITH_TYPE(
                DEBUG_TYPE_DEBUG, errs() << "- Lookup: " << var.function
                                         << " : " << var.name << "\n";
                errs()
                << "Matching value with config variable within function ["
                << (fn ? fn->getName() : "GLOBAL") << "]\n";
                errs() << "\t- Value has the name [" << value.getName()
                       << "]\n";
                errs() << "\t- Found a value name match\n\n";);
          }
        }
        if (!variable_matches && fn) {
          const MDLocalVariable *local_var = ifa->findVarNode(&value, fn);
          if (local_var &&
              (local_var->getTag() == 257 || local_var->getTag() == 256)) {
            if (local_var->getName() == var.name) {
              variable_matches = true;
              DEBUG_WITH_TYPE(
                  DEBUG_TYPE_DEBUG, errs() << "- Lookup: " << var.function
                                           << " : " << var.name << "\n";
                  errs()
                  << "Matching value with config variable within function ["
                  << fn->getName() << "]\n";
                  errs() << "\t- Variable has name [" << local_var->getName()
                         << "]\n";
                  errs() << "\t- Found a local variable match\n\n";);
            }
          }
        }

        if (variable_matches) {
          constrainValue(kind, value, var.index, var.name, var.label);
        } else if (function_matches) {
          // test if the value's content starts with match
          std::string name = ifa->getOrCreateStringFromValue(value, false);
          DEBUG_WITH_TYPE(DEBUG_TYPE_DEBUG,
                          errs() << "var.name = " << var.name << "\n";
                          errs() << "name = " << name << "\n";);
          if (name.find(var.name) == 0 && var.name.find(name) == 0) {
            errs() << "Match Detected for " << name << "\n";
            std::string meta =
                "[" + name + ":" + std::to_string(var.index) +
                "] [SrcIdx:" + std::to_string(NumSourceConstrained) + "]";
            ifa->setLabel(kind, value, var.label, gte, meta);
          }
        }
      }
    }
  }
}

void TaintAnalysisBase::fieldIndexToByteOffset(int index, const Value *v,
                                               const AbstractLoc *loc,
                                               unsigned *offset,
                                               unsigned *span) {
  if (StructType *st = convertValueToStructType(v)) {
    int nestedIndex = 0;

#if 1 // FLATTEN UNION
    int topIndex = 0;
    Type *dest = nullptr;
    for (auto subTy : st->subtypes()) {
      if (subTy->isStructTy() && subTy->getStructName().startswith("union.")) {
        subTy->dump();
        errs() << "Testing inst\n";
        v->dump();
        const Instruction *inst = dyn_cast_or_null<Instruction>(v);
        if (!inst && isa<Argument>(v)) {
          const Argument *arg = dyn_cast<Argument>(v);
          inst = &arg->getParent()->front().front();
        }
        if (inst) {
          errs() << "Confirming inst\n";
          inst = &inst->getParent()->front();
          bool foundBitCast = false, firstBBSearch = true;
          std::list<const BasicBlock *> bbs;
          std::set<const BasicBlock *> searched;
          bbs.push_back(inst->getParent());
          while (!foundBitCast && !bbs.empty()) {
            if (!firstBBSearch) {
              inst = &bbs.front()->front();
            }
            searched.insert(bbs.front());
            bbs.pop_front();
            for (auto bb : predecessors(inst->getParent())) {
              if (!searched.count(bb)) {
                bbs.push_back(bb);
              }
            }
            for (auto bb : successors(inst->getParent())) {
              if (!searched.count(bb)) {
                bbs.push_back(bb);
              }
            }
            firstBBSearch = false;
            std::list<const Instruction *> instList{inst};
            while (!instList.empty()) {
              auto curr = instList.front();
              instList.pop_front();
              // curr->dump();
              if (const BitCastInst *bit = dyn_cast<BitCastInst>(curr)) {
                if (bit->getSrcTy() == subTy ||
                    bit->getSrcTy() == subTy->getPointerTo()) {
                  dest = bit->getDestTy();
                  if (dest->isPointerTy())
                    dest = dest->getPointerElementType();
                  dest->dump();
                  int numContainedTypes = dest->getNumContainedTypes();
                  errs() << numContainedTypes << " Contained Types\n";
                  if (index > topIndex + numContainedTypes) {
                    index = index - numContainedTypes + 1;
                  } else {
                    nestedIndex = index - topIndex;
                    index = topIndex;
                  }
                  bbs.clear();
                  foundBitCast = true;
                  break;
                }
              }
              if (curr != &curr->getParent()->back()) {
                instList.push_back(curr->getNextNode());
              }
            }
          }
        }
      }
      if (index <= topIndex)
        break;
      topIndex++;
    }
#endif

    *offset = ifa->findOffsetFromFieldIndex(st, index, loc);

#if 1 // FLATTEN UNION
    if (nestedIndex >= 0 && dest && dest->isStructTy()) {
      *offset += ifa->findOffsetFromFieldIndex(dyn_cast<StructType>(dest),
                                               nestedIndex, loc);
      const DataLayout &TD = (*loc).getParentGraph()->getDataLayout();
      *span = TD.getTypeStoreSize(dest->getStructElementType(nestedIndex));
    }
#endif
  } else if (const AllocaInst *al = dyn_cast<AllocaInst>(v)) {
    if (isa<ArrayType>(al->getAllocatedType())) {
      *offset = index;
    }
  }
  return;
}

unsigned TaintAnalysisBase::getNumElements(const Value &value) {

  if (const GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(&value)) {
    return gep->getNumIndices();
  }

  Type *t = value.getType();
  // If necessary strip pointers away
  while (t->isPointerTy()) {
    t = t->getContainedType(0);
  }

  if (StructType *structType = dyn_cast<StructType>(t)) {
    return structType->getNumElements();
  } else if (ArrayType *arrayType = dyn_cast<ArrayType>(t)) {
    return arrayType->getNumElements();
  }

  return 1;
}

} // namespace deps

#endif