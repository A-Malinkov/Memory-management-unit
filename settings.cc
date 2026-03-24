/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "settings.h"

bool LogMemoryAccesses = false;
int ProcessTimeQuantum = 1000;

bool EnableTLB = false;
uint32_t TLBEntries = 32;
bool EnableASID = false;

bool EnableHole = false;
