/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "simple.h"
using namespace Simple;

#include <iostream>

/* Initialize a page table entry @entry with the given @address.
 */
static inline void
initPageTableEntry(TableEntry &entry, const uintptr_t address)
{
  entry.valid = 1;
  entry.read = 1;
  entry.ppn = address >> pageBits;
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

Simple::MMUDriver::MMUDriver()
  : pageTables(), bytesAllocated(0), kernel(nullptr)
{
}

Simple::MMUDriver::~MMUDriver()
{
  if(pageTables.empty())
    return;

  std::cerr << "MMUDriver: error: kernel did not release all page tables."
            << std::endl;
}

/* Set the host kernel of this driver to @kernel.
 */
void
Simple::MMUDriver::setHostKernel(OSKernel *kernel)
{
  this->kernel = kernel;
}

const static int entries = pageSize / sizeof(TableEntry);

/* Allocate a new page table for the process @proc.
 */
void
Simple::MMUDriver::allocatePageTable(const PID proc)
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
Simple::MMUDriver::releasePageTable(const PID proc)
{
  auto it = pageTables.find(proc);
  kernel->releaseMemory(it->second, entries * sizeof(TableEntry));
  pageTables.erase(it);
}

/* Returns the root of the page table associated with the process @proc.
 */
uintptr_t
Simple::MMUDriver::getPageTable(const PID proc)
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
Simple::MMUDriver::setMapping(const PID proc,
                              uintptr_t vAddr,
                              PhysPage &pPage)
{
  /* Ensure unused address bits are zero */
  vAddr &= (1UL << addressBits) - 1;

  int entry = vAddr / pageSize;
  initPageTableEntry(pageTables[proc][entry], pPage.addr);
}

/* Returns the number of bytes allocated for the page table.
 */
uint64_t
Simple::MMUDriver::getBytesAllocated(void) const
{
  return bytesAllocated;
}
