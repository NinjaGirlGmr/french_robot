#pragma once

#include <stdint.h>

// Minimal sys_arch definitions for NO_SYS lwIP port on Pico.
typedef uint32_t sys_prot_t;

// Allow lwIP lightweight protection macros to compile.
#define SYS_ARCH_DECL_PROTECT(level) sys_prot_t level
#define SYS_ARCH_PROTECT(level) level = sys_arch_protect()
#define SYS_ARCH_UNPROTECT(level) sys_arch_unprotect(level)
