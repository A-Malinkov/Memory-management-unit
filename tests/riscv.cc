#define BOOST_TEST_MODULE RISCV
#include <boost/test/unit_test.hpp>

#include "arch/include/riscv.h"
using namespace RISCV;

const static uint64_t MemorySize = 16 * 1024 * 1024;
const static int entries = pageSize / sizeof(TableEntry);

BOOST_AUTO_TEST_SUITE(riscv_test)

struct MMUFixture
{
  RISCV::MMU mmu;
  TableEntry *table;

  MMUFixture()
    : mmu(), table(nullptr)
  {
    table = new(std::align_val_t{pageSize}) TableEntry[entries];
    std::fill_n(table, entries, TableEntry{0});

    mmu.setPageTablePointer(reinterpret_cast<uintptr_t>(table));
  }

  ~MMUFixture()
  {
    operator delete[](table, std::align_val_t{pageSize}, std::nothrow);
  }

  MMUFixture(const MMUFixture &) = delete;
  MMUFixture &operator=(const MMUFixture &) = delete;
};

struct MMUDriverFixture
{
  RISCV::MMU mmu;
  RISCV::MMUDriver driver;

  Processor processor;
  ProcessList list = {};
  OSKernel kernel;

  MMUDriverFixture()
    : mmu(), driver(), processor(mmu),
      kernel(processor, driver, MemorySize, list)
  {
  }

  MMUDriverFixture(const MMUDriverFixture &) = delete;
  MMUDriverFixture &operator=(const MMUDriverFixture &) = delete;
};

BOOST_FIXTURE_TEST_CASE(empty_page_table, MMUFixture)
{
  MemAccess access{
    .type = MemAccessType::Load,
    .addr = 0x1234,
    .size = 8
  };
  uint64_t pAddr = 0;

  BOOST_CHECK_EQUAL(mmu.getTranslation(access, pAddr), false);
}

BOOST_FIXTURE_TEST_CASE(set_mapping_creates_translation, MMUDriverFixture)
{
  driver.allocatePageTable(0);
  processor.getMMU().setPageTablePointer(driver.getPageTable(0));

  MemAccess access{
    .type = MemAccessType::Load,
    .addr = 0x12345678,
    .size = 8
  };
  uint64_t pAddr = 0;
  BOOST_CHECK_EQUAL(mmu.getTranslation(access, pAddr), false);

  PhysPage pPage{
    .pid = 0,
    .addr = 5 * pageSize
  };
  driver.setMapping(0, access.addr & ~(pageSize - 1), pPage);

  BOOST_CHECK_EQUAL(mmu.getTranslation(access, pAddr), true);
  BOOST_CHECK_EQUAL(pAddr, pPage.addr | (access.addr & (pageSize - 1)));

  processor.getMMU().setPageTablePointer(0x0);
  driver.releasePageTable(0);
}

BOOST_FIXTURE_TEST_CASE(page_fault_adds_mapping, MMUDriverFixture)
{
  driver.allocatePageTable(0);
  processor.getMMU().setPageTablePointer(driver.getPageTable(0));

  MemAccess access{
    .type = MemAccessType::Load,
    .addr = 0x2000,
    .size = 8
  };
  uint64_t pAddr = 0;
  BOOST_CHECK_EQUAL(mmu.getTranslation(access, pAddr), false);

  mmu.processMemAccess(access);

  BOOST_CHECK_EQUAL(mmu.getTranslation(access, pAddr), true);

  kernel.releaseMemory(reinterpret_cast<void *>(pAddr), pageSize);
  processor.getMMU().setPageTablePointer(0x0);
  driver.releasePageTable(0);
}

BOOST_AUTO_TEST_SUITE_END()
