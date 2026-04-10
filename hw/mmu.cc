/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "mmu.h"
#include "settings.h"

#include <iostream>

MMU::MMU()
   //initializes an empty root, empty handler, and a TLB
  : root(0x0), pageFaultHandler(), tlb(*this, TLBEntries)
{
}
// prints out the performance metrics
MMU::~MMU()
{
  TLBStatistics &stats = tlb.stats;

  float pct = ((float) stats.hits / stats.lookups) * 100.0;
  std::cerr << std::dec << std::endl
    << "TLB statistics:"<< std::endl
    << "# lookups: " << stats.lookups << std::endl
    << "# hits: " << stats.hits << " (" << pct << "%)" << std::endl
    << "# evictions due to add: " << stats.addEvictions << std::endl
    << "# flushes: " << stats.flushes << std::endl
    << "# evictions due to flush: " << stats.flushEvictions << std::endl;
}

/* Initialize the MMU with a given page fault handler @funcForHandling.
 */
void
MMU::initialize(PageFaultFunction funcForHandling)
{
  pageFaultHandler = funcForHandling;
}

/* Set the current page table root to @_root.
 */
void
MMU::setPageTablePointer(const uintptr_t _root)
{
  root = _root;
}

/* Process a single memory access @access.
 */
void
MMU::processMemAccess(const MemAccess &access)
{
  if(root == 0x0)
    throw std::runtime_error("MMU: no page table set.");

  if(LogMemoryAccesses)
    std::cerr << "MMU: memory access: " << access << std::endl;

  uint64_t pAddr = 0x0;

  // triggers page fault if the addr is not in the memory or permissions are wrong
  while(not getTranslation(access, pAddr)){
    pageFaultHandler(access.addr);
  }

  if(LogMemoryAccesses){
    std::cerr << "MMU: translated virtual "
        << std::hex << std::showbase << access.addr
        << " to physical " << pAddr << std::endl;
  }
}

/* Returns the physical address for a memory access @access to page @pPage.
 */
uint64_t
MMU::makePhysicalAddr(const MemAccess &access, const uint64_t pPage)
{
  uint64_t pAddr = pPage << getPageBits();
  pAddr |= access.addr & (getPageSize() - 1);

  return pAddr;
}

/* Translate a memory access @access to a physical page number @pPage.
 * Returns whether the translation succeeded.
 */
bool
MMU::getTranslation(const MemAccess &access, uint64_t &pAddr)
{
  // Strip off unused sign bits in virtual address using a mask
  const uint64_t vAddr = access.addr & ((1UL << getAddressBits()) - 1);
  const uint64_t vPage = vAddr >> getPageBits();

  uint64_t pPage = 0;
  bool isWrite = (access.type == MemAccessType::Store ||
                  access.type == MemAccessType::Modify);

  /* TODO: Check if the translation is in the TLB before translating. */

  // Check the TLB
  if (EnableTLB) {
    if (tlb.lookup(vPage, pPage)) {
      // hit
      pAddr = makePhysicalAddr(access, pPage);
      return true;
    }
  }



  // TLB Miss aka do the page walk
  if (performTranslation(vPage, pPage, isWrite)) {

    // if TLB is enabled cache the new mapping
    if (EnableTLB) {
      tlb.add(vPage, pPage);
    }

    pAddr = makePhysicalAddr(access, pPage);
    return true;
  }

  // page fault
  return false;
}
