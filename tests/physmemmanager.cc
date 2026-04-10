/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#define BOOST_TEST_MODULE PhysMemManager
#include <boost/test/unit_test.hpp>

#include "os/physmemmanager.h"

const static uint64_t TestPageSize = 4096;
const static uint64_t TestMemorySize = 16 * TestPageSize;

BOOST_AUTO_TEST_SUITE(physmemmanager_test)

BOOST_AUTO_TEST_CASE(allocate_single_and_multiple_pages)
{
  PhysMemManagerHole manager(TestPageSize, TestMemorySize);

  uintptr_t first = 0x0;
  uintptr_t second = 0x0;

  BOOST_CHECK_EQUAL(manager.allocatePages(1, first), true);
  BOOST_CHECK_EQUAL(first, physMemBase);

  BOOST_CHECK_EQUAL(manager.allocatePages(3, second), true);
  BOOST_CHECK_EQUAL(second, physMemBase + TestPageSize);
}

BOOST_AUTO_TEST_CASE(release_merges_neighbors)
{
  PhysMemManagerHole manager(TestPageSize, TestMemorySize);

  uintptr_t first = 0x0;
  uintptr_t middle = 0x0;
  uintptr_t last = 0x0;

  BOOST_REQUIRE(manager.allocatePages(2, first));
  BOOST_REQUIRE(manager.allocatePages(2, middle));
  BOOST_REQUIRE(manager.allocatePages(2, last));

  manager.releasePages(first, 2);
  manager.releasePages(last, 2);
  manager.releasePages(middle, 2);

  uintptr_t merged = 0x0;
  BOOST_CHECK_EQUAL(manager.allocatePages(6, merged), true);
  BOOST_CHECK_EQUAL(merged, first);
}

BOOST_AUTO_TEST_CASE(fails_when_no_contiguous_hole_is_large_enough)
{
  PhysMemManagerHole manager(TestPageSize, 8 * TestPageSize);

  uintptr_t first = 0x0;
  uintptr_t second = 0x0;
  uintptr_t third = 0x0;
  uintptr_t addr = 0x0;

  BOOST_REQUIRE(manager.allocatePages(2, first));
  BOOST_REQUIRE(manager.allocatePages(2, second));
  BOOST_REQUIRE(manager.allocatePages(2, third));

  manager.releasePages(first, 2);
  manager.releasePages(third, 2);

  BOOST_CHECK_EQUAL(manager.allocatePages(5, addr), false);
}

BOOST_AUTO_TEST_SUITE_END()
