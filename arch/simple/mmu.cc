/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "simple.h"
using namespace Simple;

/*
 * MMU hardware. The MMU translates virtual addresses to physical addresses. It
 * does this based on mappings stored in the page table.
 */

Simple::MMU::MMU()
{
}

Simple::MMU::~MMU()
{
}

/* Translate a virtual page number @vPage to a physical page number @pPage.
 * If @isWrite is true, the translation is for a write (this is ignored).
 * Returns whether the translation succeeded.
 */
bool
Simple::MMU::performTranslation(const uint64_t vPage,
                                uint64_t &pPage,
                                bool isWrite)
{
  /* Validate alignment of page table */
  if((root & (pageTableAlign - 1)) != 0)
    throw std::runtime_error("Unaligned page table access");

  TableEntry *table = reinterpret_cast<TableEntry *>(root);
  if(not table[vPage].valid)
    return false;

  pPage = table[vPage].ppn;
  return true;
}
