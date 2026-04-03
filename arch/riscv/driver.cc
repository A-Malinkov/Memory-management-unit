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


  // Even tho the MMU ignores these for the assignment, it's still good to have them
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


RISCV::MMUDriver::MMUDriver()
  : pageTables(), bytesAllocated(0), kernel(nullptr)
{
}

RISCV::MMUDriver::~MMUDriver()
{

  // just in case we check
  if(pageTables.empty())
    return;

  std::cerr << "MMUDriver: error: kernel did not release all page tables."
            << std::endl;
}


void
RISCV::MMUDriver::setHostKernel(OSKernel *kernel)
{
  this->kernel = kernel; // reference the OS memory allocator
}

// setting up how many entries we can fit in a 4KB page
const static int entries = pageSize / sizeof(TableEntry);


void
RISCV::MMUDriver::allocatePageTable(const PID proc)
{
  // the kernel needs to allocate some RAM for the table
  TableEntry *table = reinterpret_cast<TableEntry *>
      (kernel->allocateMemory(entries * sizeof(TableEntry), pageTableAlign));

  bytesAllocated += entries * sizeof(TableEntry);

  // set all entries to invalid
  for(int i = 0; i < entries; i++){
    table[i].valid = 0;
  }

  // and lastly add the root pointer to the driver table
  pageTables.emplace(proc, table);
}


// get rid of a page table
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
  // Get the root table
  TableEntry *table = reinterpret_cast<TableEntry *>(getPageTable(proc));

  // The virtual page with the page offset removed.
  uint64_t vpn = vAddr >> pageBits;

  /*
   * The bit positions are:
   * Level 0: bits 35 to 27  (shift 27)
   * Level 1: bits 26 to 18  (shift 18)
   * Level 2: bits 17 to 9   (shift  9)
   * Level 3: bits  8 to 0   (shift  0)
   */
  for (int level = 0; level < 3; ++level)
  {
    int shift = 27 - (level * 9);
    uint64_t index = (vpn >> shift) & 0x1FF;

    // if the path doesn't exist
    if (!table[index].valid)
    {
      // Allocate a new page
      void *newPage = kernel->allocateMemory(pageSize, pageTableAlign);
      bytesAllocated += pageSize;

      // set all entries to zero in the new table
      TableEntry *nextTable = reinterpret_cast<TableEntry *>(newPage);
      for (int i = 0; i < entries; i++)
        nextTable[i].valid = 0;

      // Point the current entry to the new table
      initPageTableEntry(table[index], reinterpret_cast<uintptr_t>(newPage));

      table[index].read= 0;
      table[index].write= 0;
      table[index].execute= 0;
    }

    // go to the next table
    table = reinterpret_cast<TableEntry *>(getAddress(table[index]));
  }

  // Level 3 is the leaf entry so we need the bottom 9 bits
  uint64_t leafIndex = vpn & 0x1FF;
  initPageTableEntry(table[leafIndex], pPage.addr);
}

// self explanatory
uint64_t
RISCV::MMUDriver::getBytesAllocated(void) const
{
  return bytesAllocated;
}










