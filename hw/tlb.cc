#include "tlb.h"
#include "mmu.h"
#include "settings.h"

TLB::TLB(const MMU &mmu, const size_t max)
  : mmu(mmu), entries(), currentASID(0), max(max), stats()
{
  stats.lookups = 0;
  stats.hits = 0;
  stats.addEvictions = 0;
  stats.flushes = 0;
  stats.flushEvictions = 0;
}

TLB::~TLB() {}

bool TLB::lookup(const uint64_t vPage, uint64_t &pPage) {
  stats.lookups++;

  for (auto it = entries.begin(); it != entries.end(); ++it) {
    bool asidMatch = EnableASID ? (it->asid == currentASID) : true;

    if (it->vpn == vPage && asidMatch) {
      stats.hits++;
      pPage = it->ppn;

      // LRU: Move to front
      entries.splice(entries.begin(), entries, it);
      return true;
    }
  }
  return false;
}

void TLB::add(const uint64_t vPage, const uint64_t pPage) {
  if (max == 0) return;

  if (entries.size() >= max) {
    stats.addEvictions++;
    entries.pop_back();
  }

  // Push new entry to the front (MRU)
  entries.push_front({vPage, pPage, currentASID});
}

void TLB::flush(void) {
  // This increments the specific counter printed in the final report
  stats.flushes++;

  // Record how many valid translations we are losing
  stats.flushEvictions += entries.size();

  // Actually empty the TLB container
  entries.clear();
}

void TLB::setASID(const uintptr_t asid) {
  // Only do something if the process actually changed
  if (currentASID != asid) {
    currentASID = asid;

    // CRITICAL: If EnableASID is FALSE, it means the hardware
    // DOES NOT support tags. We must clear the TLB now.
    if (!EnableASID) {
      this->flush();
    }
  }
}