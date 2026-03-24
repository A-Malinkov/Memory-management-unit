/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "mmu.h"
#include "tlb.h"

/* TODO: Implement the TLB. */

TLB::TLB(const MMU &mmu, const size_t max)
  : mmu(mmu), max(max), stats(), currentASID(0)
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
  stats.lookups++;
  for (auto it = entries.begin(); it != entries.end(); ++it) {
    // vpn and asid check for task 3 but asid is always 0 for task 1 and 2
    if (it->vPage == vPage && it->asid == currentASID) {
      stats.hits++;
      pPage = it->pPage;

      // update most recently used entry to push it to the front
      Entry found = *it;
      entries.erase(it);
      entries.push_front(found);
      
      return true;
    }
  }

  return false;
}

/* Add a translation of @vPage to @pPage to the TLB.
 */
void
TLB::add(const uint64_t vPage, const uint64_t pPage)
{
  // if tlb full then pop the item at the back
  if (entries.size() >= max && max > 0) {
    entries.pop_back();
    stats.addEvictions++;
  }

  // add new entry to the front
  if (max > 0) {
    entries.push_front({vPage, pPage, currentASID});
  }
}

/* Flush all TLB entries.
 */
void
TLB::flush(void)
{
  stats.flushes++;
  stats.flushEvictions += entries.size();
  entries.clear();
}

/* Set the currently active ASID to @asid.
 */
void
TLB::setASID(const uintptr_t _asid)
{
  currentASID = _asid;
}
