/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "mmu.h"
#include "tlb.h"

/* TODO: Implement the TLB. */

TLB::TLB(const MMU &mmu, const size_t max)
  : mmu(mmu), max(max), stats()
{
}

TLB::~TLB()
{
}

/* Look up the virtual page number @vPage for a physical page number @pPage.
 * Returns whether the lookup was successful.
 */
bool
TLB::lookup(const uint64_t vPage, uint64_t &pPage)
{
  return false;
}

/* Add a translation of @vPage to @pPage to the TLB.
 */
void
TLB::add(const uint64_t vPage, const uint64_t pPage)
{
}

/* Flush all TLB entries.
 */
void
TLB::flush(void)
{
}

/* Set the currently active ASID to @asid.
 */
void
TLB::setASID(const uintptr_t _asid)
{
}
