/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __ARCH_RISCV__
#define __ARCH_RISCV__

#include "mmu.h"
#include "oskernel.h"

#include <map>
#include <list>
#include <cstdint>

namespace RISCV{

  /* Length of a virtual address, in bits. */
const static uint64_t addressBits = 48;

/* The page size is always a power of 2: (2^pageBits). */
const static uint64_t pageBits = 12; /* 4 KiB / page */
const static uint64_t pageSize = 1UL << pageBits;

/* Each level of the page table contains 2^(...) entries; each entry is ...
 */
const static uint64_t pageTableAlign = pageSize;


struct __attribute__((__packed__)) TableEntry
{
  uint8_t valid : 1; /* This entry is valid. */

  uint8_t read    : 1; /* Page can be read. */
  uint8_t write   : 1; /* Page can be written to. */
  uint8_t execute : 1; /* Page can be executed. */

  uint8_t user   : 1; /* This is a user mapping. */
  uint8_t global : 1; /* This mapping is global. */

  uint8_t accessed : 1; /* Page has been accessed. */
  uint16_t dirty   : 1; /* Page has been modified. */

  uint16_t reserved1 : 2;

  uint64_t ppn : 44; /* Physical page number (page offset not stored). */

  uint16_t reserved2 : 10;
};

  struct TLBEntry
  {
    uint64_t vpn; /* virtual page number. */
    uint64_t ppn; /* physical page number. */
    uint64_t asid;   /* for task 3*/
  };

/*
 * MMU hardware part.
 */

class MMU: public ::MMU
{
  private:
    std::list<TLBEntry> tlb;

    bool enableTLB = false;
    bool enableASID = false;
    size_t tlbSize = 32;
    uint64_t currentASID = 0;

    uint64_t tlbAccesses = 0;
    uint64_t tlbHits =0;

  public:
    MMU();
    virtual ~MMU();

    virtual uint8_t getPageBits(void) const override
    {
      return pageBits;
    }

    virtual uint64_t getPageSize(void) const override
    {
      return pageSize;
    }

    virtual uint8_t getAddressBits(void) const override
    {
      return addressBits;
    }

    virtual bool performTranslation(const uint64_t vPage,
                                    uint64_t &pPage,
                                    bool isWrite) override;

    void setTLBConfig(bool enable, size_t size) {
      enableTLB = enable;
      tlbSize = size;
    }

    void setASIDConfig(bool enable) {
      enableASID = enable;
    }

    void setCurrentASID(uint64_t asid) {
      currentASID = asid;

      if (!enableASID) {
        flushTLB();
      }
    

    }

    void flushTLB() {
      tlb.clear();
    }

    void printTLBstats() const;
};


/*
 * OS driver part.
 */
class MMUDriver: public ::MMUDriver
{
  protected:
    std::map<uint64_t, TableEntry *> pageTablesMap;
    uint64_t bytesAllocated;
    OSKernel *kernel; /* no ownership */

  public:
    MMUDriver();
    virtual ~MMUDriver() override;

    virtual void setHostKernel(OSKernel *kernel) override;

    virtual uint64_t getPageSize(void) const override
    {
      return pageSize;
    }

    virtual void allocatePageTable(const uint64_t PID) override;
    virtual void releasePageTable(const uint64_t PID) override;
    virtual uintptr_t getPageTable(const uint64_t PID) override;

    virtual void setMapping(const uint64_t PID,
                            uintptr_t vAddr,
                            PhysPage &pPage) override;

    virtual uint64_t getBytesAllocated(void) const override;

    /* Disallow objects from being copied, because it has a pointer member. */
    MMUDriver(const MMUDriver &driver) = delete;
    void operator=(const MMUDriver &driver) = delete;
};

} /* namespace RISCV */

#endif /* __ARCH_RISCV__ */
