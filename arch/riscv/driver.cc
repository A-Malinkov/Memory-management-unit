/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "riscv.h"
using namespace RISCV;

#include <iostream>

/* Initialize a page table entry @entry with the given @address.
 */
static inline void
initPageTableEntry(TableEntry &entry, const uintptr_t address)
{
  /* TODO: Implement. */
  // The PPN is the address divided by the page size (shifted by 12)
  entry.ppn = address >> pageBits;

  // Mark the entry as valid so the MMU hardware can use it
  entry.valid = 1;

  // Set standard permissions (Read, Write, Execute)
  // Even if the MMU ignores these for the assignment, it's good practice.
  entry.read = 1;
  entry.write = 1;
  entry.execute = 1;
}

/* Returns the address for a given page table entry @entry.
 */
static inline uintptr_t
getAddress(TableEntry &entry)
{
  return (uintptr_t) entry.ppn << pageBits;
}

/*
 * MMU driver software (part of the OS kernel). The OS kernel is in charge of
 * actually allocating and organizing the page tables for the MMU to use.
 */

RISCV::MMUDriver::MMUDriver()
  : pageTables(), bytesAllocated(0), kernel(nullptr)
{
}

RISCV::MMUDriver::~MMUDriver()
{
  if(pageTables.empty())
    return;

  std::cerr << "MMUDriver: error: kernel did not release all page tables."
            << std::endl;
}

/* Set the host kernel of this driver to @kernel.
 */
void
RISCV::MMUDriver::setHostKernel(OSKernel *kernel)
{
  this->kernel = kernel;
}

const static int entries = pageSize / sizeof(TableEntry);


/* Allocate a new page table root for the process @proc.
 */
void
RISCV::MMUDriver::allocatePageTable(const PID proc)
{
  TableEntry *table = reinterpret_cast<TableEntry *>
      (kernel->allocateMemory(entries * sizeof(TableEntry), pageTableAlign));
  /* Note: allocateMemory always allocates entire pages. */
  bytesAllocated += entries * sizeof(TableEntry);

  for(int i = 0; i < entries; i++){
    table[i].valid = 0;
  }

  /* Add to list of page table roots. */
  pageTables.emplace(proc, table);
}


/* Release the page table associated with the process @proc.
 */
void
RISCV::MMUDriver::releasePageTable(const PID proc)
{
  auto it = pageTables.find(proc);
  kernel->releaseMemory(it->second, entries * sizeof(TableEntry));
  pageTables.erase(it);
}

/* Returns the root of the page table associated with the process @proc.
 */
uintptr_t
RISCV::MMUDriver::getPageTable(const PID proc)
{
  auto kv = pageTables.find(proc);
  if(kv == pageTables.end())
    return 0x0;

  return reinterpret_cast<uintptr_t>(kv->second);
}

/* Create a new mapping for the process @proc, mapping the virtual address
 *  @vAddr to the physical page @pPage.
 */
void
RISCV::MMUDriver::setMapping(const PID proc,
                             uintptr_t vAddr,
                             PhysPage &pPage)
{
  /* Get the root table for this process. */
  TableEntry *table = reinterpret_cast<TableEntry *>(getPageTable(proc));

  /* The virtual page number (VPN) is the address with the page offset removed. */
  uint64_t vpn = vAddr >> pageBits;

  /*
   * Walk levels 0-2: these are intermediate page table levels.
   * For each level, extract the appropriate 9-bit VPN segment and
   * allocate a new sub-table if the entry is not yet valid.
   *
   * The bit positions within the VPN are:
   *   Level 0: bits 35..27  (shift 27)
   *   Level 1: bits 26..18  (shift 18)
   *   Level 2: bits 17..9   (shift  9)
   *   Level 3: bits  8..0   (shift  0) — the leaf, handled separately
   */
  for (int level = 0; level < 3; ++level)
  {
    int shift = 27 - (level * 9);
    uint64_t index = (vpn >> shift) & 0x1FF;

    if (!table[index].valid)
    {
      /* Allocate a new page to hold the next-level table. */
      void *newPage = kernel->allocateMemory(pageSize, pageTableAlign);
      bytesAllocated += pageSize;

      /* Zero-initialise all entries in the new table. */
      TableEntry *nextTable = reinterpret_cast<TableEntry *>(newPage);
      for (int i = 0; i < entries; i++)
        nextTable[i].valid = 0;

      /* Point the current entry at the newly allocated table. */
      initPageTableEntry(table[index], reinterpret_cast<uintptr_t>(newPage));

      /*
       * Intermediate entries must NOT have R/W/X set — the hardware uses
       * those bits to distinguish pointers to the next level (all zero)
       * from leaf entries (at least one of R/W/X set).
       */
      table[index].read    = 0;
      table[index].write   = 0;
      table[index].execute = 0;
    }

    /* Descend into the next-level table. */
    table = reinterpret_cast<TableEntry *>(getAddress(table[index]));
  }

  /* Level 3 — the leaf entry.  Map the VPN's bottom 9 bits to the data page. */
  uint64_t leafIndex = vpn & 0x1FF;
  initPageTableEntry(table[leafIndex], pPage.addr);
}

/* Returns the number of bytes allocated for the page table.
 */
uint64_t
RISCV::MMUDriver::getBytesAllocated(void) const
{
  return bytesAllocated;
}










