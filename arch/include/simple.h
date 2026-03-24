/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __ARCH_SIMPLE__
#define __ARCH_SIMPLE__

#include "mmu.h"
#include "oskernel.h"

#include <map>

namespace Simple{

/* Length of a virtual address, in bits. */
const static uint64_t addressBits = 48;

/* The page size is always a power of 2: (2^pageBits). */
const static uint64_t pageBits = 26; /* 64 MiB / page */
const static uint64_t pageSize = 1UL << pageBits;

/* A page table contains 2^(48 - 26) entries; each entry is 2^2 bytes.
 */
const static uint64_t pageTableAlign = 1UL << (addressBits - pageBits + 2);

struct __attribute__((__packed__)) TableEntry
{
  uint8_t valid : 1; /* This entry is valid. */

  uint8_t read  : 1; /* Page can be read. */
  uint8_t write : 1; /* Page can be written to. */

  uint16_t reserved : 7;

  uint32_t ppn : 22; /* Physical page number (page offset not stored). */
};

/*
 * MMU hardware part.
 */

class MMU: public ::MMU
{
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
};

/*
 * OS driver part.
 */
class MMUDriver: public ::MMUDriver
{
  protected:
    std::map<uint64_t, TableEntry *> pageTables;
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

} /* namespace Simple */

#endif /* __ARCH_SIMPLE__ */
