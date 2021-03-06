// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_CACHE_H_
#define LIB_CACHE_H_

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include "lib/type.h"
#include "lib/utility.h"

using std::unordered_map;
using std::string;
using std::vector;
using std::make_pair;

#define DECLARE_CACHE_CONTAINER(name, type) \
  mutable unordered_map<uint64_t, vector<type>> name##Map = {};

#define ACTION_CLEAR_ALL_CACHE(name, ...) \
  name##Map.clear();

#define DECLARE_CACHE_SET_METHODS(name, type) \
  void name##SetValueCache(uint32_t index, size_t offset, type v, size_t step) { \
    name##Map.emplace(make_pair(hashLoc(index, offset), vector<type>{v, static_cast<type>(step)})); \
  }
// create a default {} if no cache exists.
#define DECLARE_CACHE_GET_METHODS(name, type) \
  const auto& name##GetValueCache(uint32_t index, size_t offset) const { \
    return name##Map[hashLoc(index, offset)]; \
  }

#define ITERATE_IMMEDIATES_VALUE_TYPES(V) \
  V(uint8, uint8_t) \
  V(uint32, uint32_t) \
  V(uint64, uint64_t) \
  V(int32, int32_t) \
  V(int64, int64_t) \
  V(float, float) \
  V(double, double)

// keep the runtime immediates in memory, and then can be re-used.
class Cache {
 private:
  // id + offset.
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_CONTAINER)
  // meta cache container.
  mutable unordered_map<uint64_t, unordered_map<OpcodeMeta, int64_t>> metaContainer = {};
  // "memarg" cache container.
  mutable unordered_map<uint64_t, vector<uint32_t>> memargContainer = {};

  uint64_t hashLoc(uint32_t index, size_t offset) const {
    // avaiable size: 131072.
    return index * (2 << 16) + offset;
  }

 public:
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_SET_METHODS)
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_GET_METHODS)

  void int64SetMetaCache(uint32_t index, size_t offset, OpcodeMeta key, int64_t val) {
    metaContainer[hashLoc(index, offset)][key] = val;
  }

  auto int64GetMetaCache(uint32_t index, size_t offset, OpcodeMeta key) const {
    return metaContainer[hashLoc(index, offset)][key];
  }

  const auto& uint32GetMemargCache(uint32_t index, size_t offset) const {
    return memargContainer[hashLoc(index, offset)];
  }

  void uint32SetMemargCache(uint32_t index, size_t offset, uint32_t memAlign, uint32_t memOffset, size_t step) {
    memargContainer[hashLoc(index, offset)] = {memAlign, memOffset, static_cast<uint32_t>(step)};
  }

  void reset() {
    ITERATE_IMMEDIATES_VALUE_TYPES(ACTION_CLEAR_ALL_CACHE)
    metaContainer.clear();
    memargContainer.clear();
  }
};

#endif  // LIB_CACHE_H_
