//===--- PostOrderAnalysis.h - SIL POT and RPOT Analysis -------*- C++ -*--===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SILANALYSIS_POSTORDERANALYSIS_H
#define SWIFT_SILANALYSIS_POSTORDERANALYSIS_H

#include "swift/SILAnalysis/Analysis.h"
#include "swift/Basic/Range.h"
#include "swift/SIL/CFG.h"
#include "swift/SIL/SILBasicBlock.h"
#include "swift/SIL/SILFunction.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/ADT/DenseMap.h"
#include <vector>

namespace swift {

class SILBasicBlock;
class SILFunction;

class PostOrderFunctionInfo {
  std::vector<SILBasicBlock *> PostOrder;
  llvm::DenseMap<SILBasicBlock *, unsigned> BBToPOMap;

public:
  PostOrderFunctionInfo(SILFunction *F) {
    for (auto *BB : make_range(po_begin(F), po_end(F))) {
      BBToPOMap[BB] = PostOrder.size();
      PostOrder.push_back(BB);
    }
  }

  using iterator = decltype(PostOrder)::iterator;
  using const_iterator = decltype(PostOrder)::const_iterator;
  using reverse_iterator = decltype(PostOrder)::reverse_iterator;
  using const_reverse_iterator = decltype(PostOrder)::const_reverse_iterator;

  using range = iterator_range<iterator>;
  using const_range = iterator_range<const_iterator>;
  using reverse_range = iterator_range<reverse_iterator>;
  using const_reverse_range = iterator_range<const_reverse_iterator>;

  range getPostOrder() {
    return make_range(PostOrder.begin(), PostOrder.end());
  }
  const_range getPostOrder() const {
    return make_range(PostOrder.begin(), PostOrder.end());
  }
  reverse_range getReversePostOrder() {
    return make_range(PostOrder.rbegin(), PostOrder.rend());
  }
  const_reverse_range getReversePostOrder() const {
    return make_range(PostOrder.rbegin(), PostOrder.rend());
  }

  unsigned size() const { return PostOrder.size(); }

  Optional<unsigned> getPONum(SILBasicBlock *BB) const {
    auto Iter = BBToPOMap.find(BB);
    if (Iter != BBToPOMap.end())
      return Iter->second;
    return None;
  }

  Optional<unsigned> getRPONum(SILBasicBlock *BB) const {
    auto Iter = BBToPOMap.find(BB);
    if (Iter != BBToPOMap.end())
      return PostOrder.size() - Iter->second - 1;
    return None;
  }
};

/// This class is a simple wrapper around the POT iterator provided by LLVM. It
/// lazily re-evaluates the post order when it is invalidated so that we do not
/// reform the post order over and over again (it can be expensive).
class PostOrderAnalysis : public FunctionAnalysisBase<PostOrderFunctionInfo> {
protected:
  virtual PostOrderFunctionInfo *newFunctionAnalysis(SILFunction *F) override {
    return new PostOrderFunctionInfo(F);
  }

  virtual bool shouldInvalidate(SILAnalysis::PreserveKind K) override {
    return !(K & PreserveKind::Branches);
  }

public:
  PostOrderAnalysis()
      : FunctionAnalysisBase<PostOrderFunctionInfo>(AnalysisKind::PostOrder) {}

  // This is a cache and shouldn't be copied around.
  PostOrderAnalysis(const PostOrderAnalysis &) = delete;
  PostOrderAnalysis &operator=(const PostOrderAnalysis &) = delete;

  using iterator = PostOrderFunctionInfo::iterator;
  using reverse_iterator = PostOrderFunctionInfo::reverse_iterator;

  using range = PostOrderFunctionInfo::range;
  using reverse_range = PostOrderFunctionInfo::reverse_range;

  range getPostOrder(SILFunction *F) { return get(F)->getPostOrder(); }

  reverse_range getReversePostOrder(SILFunction *F) {
    return get(F)->getReversePostOrder();
  }

  // Return the size of the post order for \p F.
  unsigned size(SILFunction *F) const {
    return const_cast<PostOrderAnalysis *>(this)->get(F)->size();
  }

  static bool classof(const SILAnalysis *S) {
    return S->getKind() == AnalysisKind::PostOrder;
  }
};

} // end namespace swift

#endif
