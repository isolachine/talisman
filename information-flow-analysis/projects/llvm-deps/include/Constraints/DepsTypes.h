//===-- DepTypes.h ----------------------------------------------*- C++ -*-===//
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

#ifndef DEPSTYPES_H
#define DEPSTYPES_H

/// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
namespace deps {
enum DepsType { DT_RLConstant, DT_RLConsVar, DT_RLJoin };
}

#endif /* DEPSTYPES_H */
