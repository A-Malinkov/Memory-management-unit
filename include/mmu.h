/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __MMU_H__
#define __MMU_H__

#include <functional>

#include "process.h" /* for MemAccess */
#include "tlb.h"

using PageFaultFunction = std::function<void(uintptr_t)>;

/*
 * MMU (hardware part).
 */

class MMU
{
  protected:
    uintptr_t root;
    PageFaultFunction pageFaultHandler;
    TLB tlb;

  public:
    MMU();
    virtual ~MMU();

    void initialize(PageFaultFunction fn);
    void setPageTablePointer(const uintptr_t root);
    void processMemAccess(const MemAccess &access);

    uint64_t makePhysicalAddr(const MemAccess &access, const uint64_t pPage);
    bool getTranslation(const MemAccess &access, uint64_t &pAddr);

    TLB &getTLB()
    {
      return tlb;
    }

    /* These methods should return the architecture's constants. */
    virtual uint8_t getPageBits(void) const = 0;
    virtual uint64_t getPageSize(void) const = 0;
    virtual uint8_t getAddressBits(void) const = 0;

    /* Translate a virtual page number @vPage to a physical page number @pPage.
     * If @isWrite is true, the translation is for a write (this is ignored).
     * Returns whether the translation succeeded. */
    virtual bool performTranslation(const uint64_t vPage,
                                    uint64_t &pPage,
                                    bool isWrite) = 0;
};

#endif /* __MMU_H__ */
