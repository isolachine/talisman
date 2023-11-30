//===-- InfoflowSignature.cpp -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an interface for creating and registering information
// flow signatures. A signature registrar maintains a list of signatures.
// When a call to unknown code is encountered, the registrar invokes the
// signatures in order until a summary of the call's information flows is
// generated.
//
//===----------------------------------------------------------------------===//

#include "InfoflowSignature.h"
#include "SignatureLibrary.h"

namespace deps {

using namespace llvm;

SignatureRegistrar::SignatureRegistrar(json config) : config(config) {}

SignatureRegistrar::~SignatureRegistrar() {
  for (sig_iterator sig = sigs.begin(), end = sigs.end(); sig != end; ++sig) {
    delete (*sig);
  }
}

void SignatureRegistrar::registerSignature(const SigInfo si) {
  Signature *sig = si.makeSignature();
  if (TaintByConfig *tSig = dyn_cast<TaintByConfig>(sig)) {
    tSig->config = config;
  }
  sigs.push_back(sig);
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
SignatureRegistrar::process(const ContextID ctxt, const ImmutableCallSite cs) {
  for (sig_iterator sig = sigs.begin(), end = sigs.end(); sig != end; ++sig) {
    if ((*sig)->accept(ctxt, cs)) {
      return (*sig)->process(ctxt, cs);
    }
  }
  assert(false && "No signature matched call site.");
  return std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>();
}

} // namespace deps
