//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef ____LIBUNWIND_CONFIG_H__
#define ____LIBUNWIND_CONFIG_H__

#define _LIBUNWIND_VERSION 240000

#if defined(__arm__) && !defined(__USING_SJLJ_EXCEPTIONS__) && \
    !defined(__ARM_DWARF_EH__) && !defined(__SEH__)
#define _LIBUNWIND_ARM_EHABI
#endif

#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_X86       8
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_X86_64    32
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_PPC       112
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_PPC64     116
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_ARM64     95
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_ARM       287
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_OR1K      32
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_MIPS      65
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_SPARC     31
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_SPARC64   31
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_HEXAGON   34
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_RISCV     64
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_VE        143
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_S390X     83
#define _LIBUNWIND_HIGHEST_DWARF_REGISTER_LOONGARCH 64

# define _LIBUNWIND_TARGET_I386
# define _LIBUNWIND_TARGET_X86_64 1
# define _LIBUNWIND_CONTEXT_SIZE 167
# define _LIBUNWIND_CURSOR_SIZE 204
# define _LIBUNWIND_HIGHEST_DWARF_REGISTER 287

#if defined(__has_feature)
#  if __has_feature(ptrauth_calls) && __has_feature(ptrauth_returns)
#    define _LIBUNWIND_TARGET_AARCH64_AUTHENTICATED_UNWINDING 1
#  elif __has_feature(ptrauth_calls) != __has_feature(ptrauth_returns)
#    error "Either both or none of ptrauth_calls and ptrauth_returns "\
           "is allowed to be enabled"
#  endif
#endif

#endif // ____LIBUNWIND_CONFIG_H__
