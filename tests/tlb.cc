#define BOOST_TEST_MODULE TLB
#include <boost/test/unit_test.hpp>

#include "mmu.h"
#include "settings.h"

class DummyMMU: public MMU
{
  public:
    virtual uint8_t getPageBits(void) const override
    {
      return 12;
    }

    virtual uint64_t getPageSize(void) const override
    {
      return 1UL << 12;
    }

    virtual uint8_t getAddressBits(void) const override
    {
      return 48;
    }

    virtual bool performTranslation(const uint64_t,
                                    uint64_t &,
                                    bool) override
    {
      return false;
    }
};

struct SettingsGuard
{
  bool oldEnableTLB;
  bool oldEnableASID;

  SettingsGuard()
    : oldEnableTLB(EnableTLB), oldEnableASID(EnableASID)
  {
  }

  ~SettingsGuard()
  {
    EnableTLB = oldEnableTLB;
    EnableASID = oldEnableASID;
  }
};

BOOST_AUTO_TEST_SUITE(tlb_test)

/* test adding and lookng up entries*/
BOOST_AUTO_TEST_CASE(add_and_lookup_hit)
{
  SettingsGuard guard;
  DummyMMU mmu;
  TLB tlb(mmu, 2);

  uint64_t pPage = 0;
  tlb.add(3, 99);

  BOOST_CHECK_EQUAL(tlb.lookup(3, pPage), true);
  BOOST_CHECK_EQUAL(pPage, 99);
  BOOST_CHECK_EQUAL(tlb.stats.lookups, 1);
  BOOST_CHECK_EQUAL(tlb.stats.hits, 1);
}

/* test flushing entries*/
BOOST_AUTO_TEST_CASE(flush_removes_entries)
{
  SettingsGuard guard;
  DummyMMU mmu;
  TLB tlb(mmu, 2);

  uint64_t pPage = 0;
  tlb.add(1, 11);
  tlb.add(2, 22);
  tlb.flush();
  BOOST_CHECK_EQUAL(tlb.lookup(1, pPage), false);
  BOOST_CHECK_EQUAL(tlb.stats.flushes, 1);
  BOOST_CHECK_EQUAL(tlb.stats.flushEvictions, 2);
}

/* test asid seperation */
BOOST_AUTO_TEST_CASE(asid_separates_entries)
{
  SettingsGuard guard;
  EnableASID = true;

  DummyMMU mmu;
  TLB tlb(mmu, 4);
  uint64_t pPage = 0;

  tlb.setASID(1);
  tlb.add(7, 70);
  tlb.setASID(2);
  BOOST_CHECK_EQUAL(tlb.lookup(7, pPage), false);
  tlb.setASID(1);
  BOOST_CHECK_EQUAL(tlb.lookup(7, pPage), true);
  BOOST_CHECK_EQUAL(pPage, 70);
}

BOOST_AUTO_TEST_SUITE_END()
