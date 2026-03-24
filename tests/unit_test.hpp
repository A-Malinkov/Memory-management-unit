/* pagetables -- A framework to experiment with memory management
 *
 * Unit tests for the RISC-V Sv48 MMU and MMU driver.
 * Place this file in the tests/ directory, then run: make check
 */
 
#define BOOST_TEST_MODULE RISCVTests
#include <boost/test/unit_test.hpp>
 
#include "arch/include/riscv.h"
#include "include/oskernel.h"
#include "include/processor.h"


struct RISCVFixture
{
  static constexpr uint64_t MEM_SIZE = 32ULL * 1024 * 1024; /* 32 MiB */
 
  RISCV::MMU       mmu;
  RISCV::MMUDriver driver;
  ProcessList      processList;  
  Processor        processor;
  OSKernel         kernel;
 
  PID pid;
 
  RISCVFixture()
    : mmu(),
      driver(),
      processList(),
      processor(mmu),
      kernel(processor, driver, MEM_SIZE, processList),
      pid(reinterpret_cast<PID>(this))
  {
    mmu.initialize([this](uintptr_t faultAddr) {
      kernel.pageFaultHandler(faultAddr);
    });
 
    driver.allocatePageTable(pid);
    mmu.setPageTablePointer(driver.getPageTable(pid));
  }
 
  ~RISCVFixture()
  {
    driver.releasePageTable(pid);
  }
 

  uintptr_t manualMap(uint64_t vAddr)
  {
    uint64_t pageBase = vAddr & ~(RISCV::pageSize - 1);
 
    void *mem = kernel.allocateMemory(RISCV::pageSize, RISCV::pageTableAlign);
    BOOST_REQUIRE_MESSAGE(mem != nullptr, "physical page allocation failed");
 
    PhysPage pp;
    pp.pid  = pid;
    pp.addr = reinterpret_cast<uintptr_t>(mem); 
    driver.setMapping(pid, pageBase, pp);
 
    return pp.addr;
  }
 

  bool translate(uint64_t vAddr, uint64_t &pAddr)
  {
    MemAccess acc;
    acc.type = MemAccessType::Load;
    acc.addr = vAddr;
    acc.size = 4;
    return mmu.getTranslation(acc, pAddr);
  }
};
 
 
BOOST_AUTO_TEST_SUITE(table_entry_constants)
 
/* Each PTE is a 64-bit word — must be exactly 8 bytes. */
BOOST_AUTO_TEST_CASE(table_entry_is_8_bytes)
{
  BOOST_CHECK_EQUAL(sizeof(RISCV::TableEntry), 8u);
}
 
/* Sv48 uses 4 KiB pages → pageBits = 12. */
BOOST_AUTO_TEST_CASE(page_bits_are_12)
{
  BOOST_CHECK_EQUAL(RISCV::pageBits, 12u);
}
 
/* pageSize = 2^pageBits = 4096. */
BOOST_AUTO_TEST_CASE(page_size_is_4096)
{
  BOOST_CHECK_EQUAL(RISCV::pageSize, 4096u);
}
 
/* Sv48 virtual addresses are 48 bits wide. */
BOOST_AUTO_TEST_CASE(address_space_is_48_bits)
{
  RISCV::MMU mmu;
  BOOST_CHECK_EQUAL(mmu.getAddressBits(), 48u);
  BOOST_CHECK_EQUAL(mmu.getPageBits(),    12u);
  BOOST_CHECK_EQUAL(mmu.getPageSize(),  4096u);
}
 
/* Each level of the page table holds 512 entries (9 index bits). */
BOOST_AUTO_TEST_CASE(entries_per_level)
{
  uint64_t entriesPerLevel = RISCV::pageSize / sizeof(RISCV::TableEntry);
  BOOST_CHECK_EQUAL(entriesPerLevel, 512u);
}
 
BOOST_AUTO_TEST_SUITE_END()
 
/* =========================================================================
 * Suite 2: Driver — page table management
 * ========================================================================= */
 
BOOST_AUTO_TEST_SUITE(driver_page_table_management)
 
/* A freshly allocated page table has a non-zero root address. */
BOOST_AUTO_TEST_CASE(allocate_gives_nonzero_root)
{
  RISCVFixture f;
  BOOST_CHECK_NE(f.driver.getPageTable(f.pid), 0u);
}
 
/* Allocating a page table costs at least one page (the root). */
BOOST_AUTO_TEST_CASE(allocate_charges_bytes)
{
  RISCVFixture f;
  BOOST_CHECK_GE(f.driver.getBytesAllocated(), RISCV::pageSize);
}
 
/* After releasing a page table, getPageTable returns 0. */
BOOST_AUTO_TEST_CASE(release_removes_root)
{
  RISCVFixture f;
  PID pid2 = f.pid + 0xDEAD;
  f.driver.allocatePageTable(pid2);
  BOOST_REQUIRE_NE(f.driver.getPageTable(pid2), 0u);
 
  f.driver.releasePageTable(pid2);
  BOOST_CHECK_EQUAL(f.driver.getPageTable(pid2), 0u);
}
 
/* Two processes get different root addresses. */
BOOST_AUTO_TEST_CASE(two_processes_get_different_roots)
{
  RISCVFixture f;
  PID pid2 = f.pid + 0x1;
  f.driver.allocatePageTable(pid2);
 
  uintptr_t root1 = f.driver.getPageTable(f.pid);
  uintptr_t root2 = f.driver.getPageTable(pid2);
  BOOST_CHECK_NE(root1, root2);
 
  f.driver.releasePageTable(pid2);
}
 
BOOST_AUTO_TEST_SUITE_END()
 