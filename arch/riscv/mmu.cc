/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "riscv.h"
#include "settings.h"
#include <iostream>

using namespace RISCV;

RISCV::MMU::MMU()
{
}

RISCV::MMU::~MMU()
{
}

bool
RISCV::MMU::performTranslation(const uint64_t vPage, uint64_t &pPage, bool isWrite)
{
    // 1. TLB LOOKUP
    // Use the global EnableTLB from settings.h
    if (EnableTLB) {
        if (tlb.lookup(vPage, pPage)) {
            return true; // HIT: Stats are updated inside tlb.lookup()
        }
    }

    // 2. PAGE TABLE WALK
    uint64_t currentTableAddr = root;
    for (int level = 0; level < 4; ++level) {
        int shift = 27 - (level * 9);
        uint64_t index = (vPage >> shift) & 0x1FF;

        const TableEntry *table = reinterpret_cast<const TableEntry *>(currentTableAddr);
        const TableEntry &entry = table[index];

        if (!entry.valid) return false;

        if (level == 3) {
            pPage = entry.ppn;

            // 3. TLB UPDATE
            // If the walk succeeded, we MUST add it to the TLB
            if (EnableTLB) {
                tlb.add(vPage, pPage); // Stats are updated inside tlb.add()
            }
            return true;
        }
        currentTableAddr = static_cast<uint64_t>(entry.ppn) * pageSize;
    }
    return false;
}


