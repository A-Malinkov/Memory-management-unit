#ifndef __TLB_H__
#define __TLB_H__

#include <cstdint>
#include <cstddef>
#include <list>

struct TLBStatistics {
  int lookups;
  int hits;
  int addEvictions;
  int flushes;
  int flushEvictions;
};

class MMU;

class TLB {
  public:
    /* Define Entry inside the class for clarity */
    struct Entry {
        uint64_t vpn;
        uint64_t ppn;
        uintptr_t asid;
    };

  protected:
    const MMU &mmu;
    /* Reordered to match constructor initialization order */
    std::list<Entry> entries;
    uintptr_t currentASID;
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

#endif