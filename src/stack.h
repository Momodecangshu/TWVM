// Copyright 2019 YHSPY. All rights reserved.
#ifndef STACK_H_
#define STACK_H_

#include <stack>
#include <memory>
#include <cstring>
#include <iostream>
#include <type_traits>
#include "src/instances.h"
#include "src/constants.h"
#include "src/utils.h"
#include "src/macros.h"
#include "src/types.h"

using std::stack;
using std::shared_ptr;
using std::is_same;
using std::ostream;
using std::dec;
using std::memcmp;
using std::memcpy;

struct WasmFuncInstance;

class ValueFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(ValueFrameTypes type) : bitPattern{} {}
  ValueFrame(const ValueFrame *other) : type(other->type), isValueZero(other->isValueZero) {
    // copy "bitPattern";
    memcpy(bitPattern, other->bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

#define DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS(name, localtype, ctype) \
  ValueFrame(ctype v) : type(localtype), bitPattern{} { \
    isValueZero = (v == static_cast<ctype>(0)); \
    Utils::writeUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern), v); \
  } \
  const ctype to##name() { \
    return Utils::readUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern)); \
  } 
  ITERATE_WASM_VAL_TYPE(DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS)

  const bool operator==(const ValueFrame& other) {
    return type == other.type && !memcmp(bitPattern, other.bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

  inline ValueTypesCode getValueType() {
    return static_cast<ValueTypesCode>(type);
  }

  template <typename T>
  inline void resetValue(T v) {
    if (is_same<T, int32_t>::value) {
      type = ValueFrameTypes::kI32Value;
    } else if (is_same<T, int64_t>::value) {
      type = ValueFrameTypes::kI64Value;
    } else if (is_same<T, float>::value) {
      type = ValueFrameTypes::kF32Value;
    } else if (is_same<T, double>::value) {
      type = ValueFrameTypes::kF64Value;
    } else {
      Utils::report("invalid type of \"ValueFrame\"!");
    }
    Utils::writeUnalignedValue<T>(reinterpret_cast<uintptr_t>(bitPattern), v);
  }  

  void outputValue(ostream &out) {
    switch (type) {
      case ValueFrameTypes::kF32Value: { out << toF32(); break; }
      case ValueFrameTypes::kF64Value: { out << toF64(); break; }
      case ValueFrameTypes::kI32Value: { out << toI32(); break; }
      case ValueFrameTypes::kI64Value: { out << toI64(); break; }
      default: break;
    }
  }

  const auto isZero() {
    return isValueZero;
  }

 private:
  ValueFrameTypes type;
  bool isValueZero;
  uint8_t bitPattern[WASM_VALUE_BIT_PATTERN_WIDTH];
};

class LabelFrame {
 public:
  SET_STRUCT_MOVE_ONLY(LabelFrame);
  shared_ptr<PosPtr> end;
  shared_ptr<PosPtr> branch;
  shared_ptr<PosPtr> start;

  LabelFrame(
    ValueTypesCode resultType, 
    size_t valueStackHeight) : resultType(resultType), valueStackHeight(valueStackHeight) {}

 private:
  // for "block", "loop" and "if";
  ValueTypesCode resultType;
  // determine the # of returning arity;
  const size_t valueStackHeight = 0;
};

class ActivationFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ActivationFrame);
  const WasmFuncInstance *pFuncIns = nullptr;
  vector<ValueFrame> locals = {};

  ActivationFrame(
    const WasmFuncInstance *pFuncIns, 
    size_t valueStackHeight, 
    size_t labelStackHeight, 
    vector<ValueFrame> locals = {}) : 
    pFuncIns(pFuncIns), valueStackHeight(valueStackHeight), labelStackHeight(labelStackHeight), locals(locals) {};
  
  inline auto getValueStackHeight() { return valueStackHeight; }
  inline auto getLabelStackHeight() { return labelStackHeight; }
 private:
  // determine the # of returning arity;
  const size_t valueStackHeight = 0;
  // determine whether we reach the "end" of the function;
  const size_t labelStackHeight = 0;
};

// for saving "Values" / "Labels" / "Activations";
class Stack {
 public:
  const bool checkStackState(bool startEntry = true) {
    // check the status of stack;
    const auto leftValueSize = valueStack.size();
    Utils::say() << "(" << (startEntry ? "start" : "main") << "): ";
    if (leftValueSize == 1) {
      valueStack.top().outputValue(cout << dec);
      valueStack.pop();
    } else {
      cout << "(void)";
    }
    cout << endl;
    return leftValueSize <= 1;
  }

  // in order to reduce the overhead from casting between parent and child types -
  // caused by "dynamic_cast" and "static_cast", we'd better store these three kinds of Frames -
  // separately.
  stack<ValueFrame> valueStack;
  stack<LabelFrame> labelStack;
  stack<ActivationFrame> activationStack;
  // for temporary saving arity, due to the lack of random accessing of the stack;
  stack<ValueFrame> tempValueStack;
};

#endif  // STACK_H_
