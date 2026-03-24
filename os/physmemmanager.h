/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __PHYSMEMMANAGER_H__
#define __PHYSMEMMANAGER_H__

#include "settings.h"

#include <vector>
#include <cstddef>

/* We assume that the RAM starts at 16GiB in the physical address space.
 */
const static uint64_t physMemBase = 16UL * 1024 * 1024 * 1024;

/*
 * Physical memory manager, base class.
 */

class PhysMemManager
{
  protected:
    void *baseAddress;
    const uint64_t pageSize;
    const uint64_t memorySize;

    uint64_t nPages;
    uint64_t nAllocatedPages;
    uint64_t maxAllocatedPages;

  public:
    PhysMemManager(const uint64_t pageSize, const uint64_t memorySize);
    virtual ~PhysMemManager();

    virtual bool allocatePages(size_t count, uintptr_t &addr) = 0;
    virtual void releasePages(uintptr_t addr, size_t count) = 0;

    bool allReleased(void) const;
    uint64_t getMaxAllocatedPages(void) const;

    PhysMemManager(const PhysMemManager &) = delete;
    PhysMemManager &operator=(const PhysMemManager &) = delete;
};

/*
 * Physical memory manager, bitmap version.
 */

class PhysMemManagerBitmap: public PhysMemManager
{
  protected:
    std::vector<bool> allocated;

  public:
    PhysMemManagerBitmap(const uint64_t pageSize, const uint64_t memorySize);

    bool allocatePages(size_t count, uintptr_t &addr);
    void releasePages(uintptr_t addr, size_t count);
};

/*
 * Physical memory manager, hole list version.
 */

class PhysMemManagerHole: public PhysMemManager
{
  protected:
    /* TODO: Add data structures necessary for hole list. */

  public:
    PhysMemManagerHole(const uint64_t pageSize, const uint64_t memorySize);

    bool allocatePages(size_t count, uintptr_t &addr);
    void releasePages(uintptr_t addr, size_t count);
};

#endif /* __PHYSMEMMANAGER_H__ */
