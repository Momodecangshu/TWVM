// Copyright 2019 YHSPY. All rights reserved.
#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <iostream>
#include <memory>
#include "src/types.h"
#include "src/utilities.h"

using std::vector;
using std::shared_ptr;

// pay attention to the bound check;
#define WRAP_SELECT_METHOD(name, key) \
  inline const auto name() { return &key; } \
  inline const auto name(uint32_t index) \
    { return (index >= 0 && index < key.size()) ? &key[index] : nullptr; }

class Module {
 public:
  Module() = default;
  ~Module() {
    Utilities::reportDebug("module has been destructed.");
  }

  void setModContent(const vector<uchar_t> &content) {
    buf = content.data();
    contentLength = content.size();
  }

  inline size_t getModContentLength(void) {
    return contentLength;
  }

  inline const uchar_t* getCurrentOffsetBuf(void) {
    return (buf + p);
  }

  inline void increaseBufOffset(size_t step) {
    p += step;
  }

  inline bool hasEnd() {
    return contentLength == p;
  }

  WRAP_SELECT_METHOD(getTable, tables)
  WRAP_SELECT_METHOD(getFunctionSig, funcSignatures)
  WRAP_SELECT_METHOD(getFunction, functions)
  WRAP_SELECT_METHOD(getExport, exportTable)
  WRAP_SELECT_METHOD(getImport, importTable)
  WRAP_SELECT_METHOD(getGlobal, globals)
  WRAP_SELECT_METHOD(getElement, elements)

  inline auto& getMemory() { return memory; }
  inline auto& getImportedFuncCount() { return importedFuncCount; }
  inline auto& getImportedTableCount() { return importedTableCount; }
  inline auto& getStartFuncIndex() { return startFuncIndex; }

 private:
  const uchar_t *buf;
  size_t contentLength;
  // start from the first section;
  size_t p = 8;
  size_t importedFuncCount = 0;
  size_t importedTableCount = 0;
  size_t startFuncIndex = -1;
  // params, returns;
  vector<WasmFunctionSig> funcSignatures;
  // order: (external imported) -> (internal defined);
  vector<WasmFunction> functions;
  vector<WasmTable> tables;
  shared_ptr<WasmMemory> memory = nullptr;
  vector<WasmGlobal> globals;
  vector<WasmExport> exportTable;
  vector<WasmImport> importTable;
  vector<WasmElement<>> elements;
};

using shared_module_t = shared_ptr<Module>;

#endif  // MODULE_H_