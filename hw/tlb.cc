/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "mmu.h"
#include "tlb.h"
#include "settings.h"
#include <list>
/* TODO: Implement the TLB. */
struct TLBEntry {
    uint64_t vpn;
    uint64_t ppn;
    uintptr_t asid;
};

static std::list<TLBEntry> entries;
static uintptr_t currentASID = 0;
TLB::TLB(const MMU &mmu, const size_t max)
  : mmu(mmu), max(max), stats()
{
/* Initialize all statistics to zero  */
  stats.lookups = 0;
  stats.hits = 0;
  stats.addEvictions = 0;
  stats.flushes = 0;
  stats.flushEvictions = 0;
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
  stats.lookups++; // Increment every time a lookup is attempted

  for (auto it = entries.begin(); it != entries.end(); ++it) {
    bool asidMatch = EnableASID ? (it->asid == currentASID) : true;

    if (it->vpn == vPage && asidMatch) {
      stats.hits++; // Increment only on a successful match
      pPage = it->ppn;

      // LRU: Move to front
      entries.splice(entries.begin(), entries, it);
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
  /* If the TLB is full, evict the least recently used entry  */
  if (entries.size() >= max) {
    stats.addEvictions++; // Increment when forced to kick an entry out to make room
    entries.pop_back();
  }

  entries.push_front({vPage, pPage, currentASID});
}

/* Flush all TLB entries.
 */
void
TLB::flush(void)
{
  stats.flushes++; // Increment the number of times a flush was triggered
  stats.flushEvictions += entries.size(); // Count how many valid entries were cleared

  entries.clear();
}

/* Set the currently active ASID to @asid.
 */
void
TLB::setASID(const uintptr_t _asid)
{
  currentASID = _asid;
}
