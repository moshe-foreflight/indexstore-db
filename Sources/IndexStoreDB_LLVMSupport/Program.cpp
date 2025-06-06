//===-- Program.cpp - Implement OS Program Concept --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Program concept.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/llvm_Support_Program.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/llvm_Config_llvm-config.h>
#include <system_error>
using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

static bool Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args, Optional<ArrayRef<StringRef>> Env,
                    ArrayRef<Optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *ErrMsg);

int sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        Optional<ArrayRef<StringRef>> Env,
                        ArrayRef<Optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *ErrMsg, bool *ExecutionFailed) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  if (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg)) {
    if (ExecutionFailed)
      *ExecutionFailed = false;
    ProcessInfo Result = Wait(
        PI, SecondsToWait, /*WaitUntilTerminates=*/SecondsToWait == 0, ErrMsg);
    return Result.ReturnCode;
  }

  if (ExecutionFailed)
    *ExecutionFailed = true;

  return -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               Optional<ArrayRef<StringRef>> Env,
                               ArrayRef<Optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *ErrMsg,
                               bool *ExecutionFailed) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  if (ExecutionFailed)
    *ExecutionFailed = false;
  if (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg))
    if (ExecutionFailed)
      *ExecutionFailed = true;

  return PI;
}

bool sys::commandLineFitsWithinSystemLimits(StringRef Program,
                                            ArrayRef<const char *> Args) {
  SmallVector<StringRef, 8> StringRefArgs;
  StringRefArgs.reserve(Args.size());
  for (const char *A : Args)
    StringRefArgs.emplace_back(A);
  return commandLineFitsWithinSystemLimits(Program, StringRefArgs);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Program.inc"
#endif
#ifdef _WIN32
#include "Windows/Program.inc"
#endif
