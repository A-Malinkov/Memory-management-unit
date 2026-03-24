/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <cstdint>

/* These settings can be changed with command-line options. */
extern bool LogMemoryAccesses;
extern int ProcessTimeQuantum;

extern bool EnableTLB;
extern uint32_t TLBEntries;
extern bool EnableASID;

extern bool EnableHole;

#endif /* __SETTINGS_H__ */
