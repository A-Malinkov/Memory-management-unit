/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "riscv.h"
using namespace RISCV;

/*
 * MMU hardware. The MMU translates virtual addresses to physical addresses. It
 * does this based on mappings stored in the page table.
 */

RISCV::MMU::MMU()
{
}

RISCV::MMU::~MMU()
{
}

/* Translate a virtual page number @vPage to a physical page number @pPage.
 * If @isWrite is true, the translation is for a write (this is ignored).
 * Returns whether the translation succeeded.
 */
bool
RISCV::MMU::performTranslation(const uint64_t vPage,
                                uint64_t &pPage,
                                bool isWrite)
{
  // our start is the root
  uint64_t currentTableAddr = root;

  for (int level = 0; level < 4; ++level)
  {
    // get the 9-bit index for cur level
    int shift = 27 - (level * 9);
    uint64_t index = (vPage >> shift) & 0x1FF;


    const TableEntry *table = reinterpret_cast<const TableEntry *>(currentTableAddr);
    const TableEntry &entry = table[index];

    // this is how we trigger a page fault
    // If the entry is not valid
    if (!entry.valid)
      return false;

    if (level == 3)
    {
      // Leaf entry aka the PPN is the physical page number
      pPage = entry.ppn;
      return true;
    }
    else
    {

      //  Intermediate entry ska the PPN points to the next-level
      //  Convert the page number to a byte address
      currentTableAddr = static_cast<uint64_t>(entry.ppn) * pageSize;
    }
  }

  // I don't think we can get here but we need to return something
  return false;
}










