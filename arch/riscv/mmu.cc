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
  /* Start at the root of the page table tree. */
  uint64_t currentTableAddr = root;

  for (int level = 0; level < 4; ++level)
  {
    /* Extract the 9-bit index for this level. */
    int shift = 27 - (level * 9);
    uint64_t index = (vPage >> shift) & 0x1FF;

    /* Read the entry from the current table. */
    const TableEntry *table = reinterpret_cast<const TableEntry *>(currentTableAddr);
    const TableEntry &entry = table[index];

    /* If the entry is not valid, signal a page fault. */
    if (!entry.valid)
      return false;

    if (level == 3)
    {
      /* Leaf entry: the PPN is the physical page number of the data page. */
      pPage = entry.ppn;
      return true;
    }
    else
    {
      /*
       * Intermediate entry: the PPN points to the next-level table.
       * Convert the page number to a byte address using pageSize.
       */
      currentTableAddr = static_cast<uint64_t>(entry.ppn) * pageSize;
    }
  }

  /* Should never be reached. */
  return false;
}










