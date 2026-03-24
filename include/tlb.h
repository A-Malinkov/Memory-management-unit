/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __TLB_H__
#define __TLB_H__

/* Structure representing statistics for the TLB. */
struct TLBStatistics
{
  int lookups; /* Amount of lookups */
  int hits;    /* Amount of hits */

  int addEvictions;   /* Amount of evictions due to add */

  int flushes; /* Amount of flushes */
  int flushEvictions; /* Amount of evictions due to flush */
};

class MMU;

/*
 * Translation lookaside buffer.
 */

class TLB
{
  protected:
    /* Reference to MMU; to be filled by initializer list in constructor */
    const MMU &mmu;

    /* Maximum number of entries in TLB */
    const size_t max;

  public:
    TLB(const MMU &mmu, const size_t max);
    ~TLB();

    TLBStatistics stats;

    bool lookup(const uint64_t vPage, uint64_t &pPage);
    void add(const uint64_t vPage, const uint64_t pPage);
    void flush(void);

    void setASID(const uintptr_t asid);
};

#endif /* __TLB_H__ */
