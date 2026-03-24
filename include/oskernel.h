/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __OSKERNEL_H__
#define __OSKERNEL_H__

#include "processor.h"
#include "process.h"

class OSKernel;
class PhysMemManager;

/* Structure representing a physical page allocated to a process. */
struct PhysPage
{
  PID pid;
  uintptr_t addr; /* Full address of physical page. */
};

/*
 * MMU driver (software part).
 */

class MMUDriver
{
  public:
    MMUDriver();
    virtual ~MMUDriver();

    /* Set the host kernel of this driver to @kernel. */
    virtual void setHostKernel(OSKernel *kernel) = 0;

    /* Returns the page size for this driver. */
    virtual uint64_t getPageSize(void) const = 0;

    /* Allocate a new page table (root) for the process @proc. */
    virtual void allocatePageTable(const PID proc) = 0;

    /* Release the page table associated with the process @proc. */
    virtual void releasePageTable(const PID proc) = 0;

    /* Returns the root of the page table associated with the process @proc. */
    virtual uintptr_t getPageTable(const PID proc) = 0;

    /* Create a new mapping for the process @proc, mapping the virtual address
     *  @vAddr to the physical page @pPage. */
    virtual void setMapping(const PID proc,
                            uintptr_t vAddr,
                            PhysPage &pPage) = 0;

    /* Returns the number of bytes allocated for the page table. */
    virtual uint64_t  getBytesAllocated(void) const = 0;
};

/* The OS kernel is only concerned with allocating physical memory and creating
 * page table mappings in response to page faults. We do not deal with page
 * permissions, virtual address space maps, and so on to keep things simple.
 */
class OSKernel
{
  protected:
    std::unique_ptr<PhysMemManager> manager;
    Processor &processor;
    MMUDriver &driver;

    ProcessList &processList;
    std::shared_ptr<Process> current;

    int nPageFaults;
    int nContextSwitches;

    void logPageFault(const uint64_t faultAddr);
    void logPageMapping(const uint64_t vAddr,
                        const uint64_t pAddr);

  public:
    OSKernel(Processor &processor, MMUDriver &driver, uint64_t memorySize,
             ProcessList &processList);
    virtual ~OSKernel();

    void *allocateMemory(size_t size, uint64_t align);
    void releaseMemory(void *ptr, size_t size);

    void terminateProcess(const PID proc);

    void pageFaultHandler(const uint64_t faultAddr);
    void interruptHandler(InterruptRequest request);

    /* Getters for statistics */
    int getNPageFaults() const;
    int getNContextSwitches() const;
    int getMaxAllocatedPages() const;
};

#endif /* __OSKERNEL_H__ */
