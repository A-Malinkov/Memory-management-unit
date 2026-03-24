# pagetables -- A framework to experiment with memory management
#
# Copyright (C) 2017--2026 Leiden University, The Netherlands.
CXX = c++
CXXFLAGS = -Wall -Weffc++ -std=c++17 -Iinclude -g -O3
# Fix build with newer C++ compilers <https://stackoverflow.com/a/77503976>
CXXFLAGS += -DBOOST_NO_CXX98_FUNCTION_BASE

LDFLAGS = -lz

BOOST_CXXFLAGS = -DBOOST_TEST_DYN_LINK
BOOST_LIBS = -lboost_unit_test_framework

HEADERS = \
	include/settings.h \
	include/exceptions.h \
	include/process.h \
	include/mmu.h \
	include/tlb.h \
	include/oskernel.h \
	include/processor.h

HW_OBJS = \
	hw/mmu.o \
	hw/tlb.o \
	hw/processor.o

OS_HEADERS = \
	os/tracereader.h \
	os/linereader.h \
	os/physmemmanager.h

OS_OBJS = \
	os/tracereader.o \
	os/linereader.o \
	os/physmemmanager.o \
	os/process.o \
	os/oskernel.o

ARCH_HEADERS = \
	arch/include/simple.h \
	arch/include/riscv.h

ARCH_OBJS = \
	arch/simple/driver.o \
	arch/simple/mmu.o \
	arch/riscv/driver.o \
	arch/riscv/mmu.o

OBJS = \
	main.o \
	settings.o \
	$(HW_OBJS)

# Unit tests
# This will automatically make all .cc files in the tests directory.
TESTDIR ?= tests
TESTS = $(patsubst %.cc,%,$(wildcard $(TESTDIR)/*.cc))

TEST_HEADERS = \
	$(HEADERS) \
	$(OS_HEADERS) \
	$(ARCH_HEADERS)

TEST_OBJS = \
	$(HW_OBJS) \
	$(OS_OBJS) \
	$(ARCH_OBJS) \
	settings.o

.PHONY: all check clean

all: pagetables tests

tests: $(TESTS)

pagetables: $(OS_OBJS) $(OBJS) $(ARCH_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cc $(HEADERS) $(ARCH_HEADERS)
	$(CXX) $(CXXFLAGS) -Iarch/include -o $@ -c $<

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OS_OBJS): %.o: %.cc $(HEADERS) $(OS_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(ARCH_OBJS): %.o: %.cc $(HEADERS) $(ARCH_HEADERS)
	$(CXX) $(CXXFLAGS) -Iarch/include -o $@ -c $<

$(TESTDIR)/%: $(TESTDIR)/%.cc $(TEST_OBJS) $(TEST_HEADERS)
	$(CXX) $(CXXFLAGS) -I. $(BOOST_CXXFLAGS) -o $@ $< \
	       $(TEST_OBJS) $(BOOST_LIBS) $(LDFLAGS)

check: $(TESTS)
	@for i in $(TESTS); do ./$$i; done

clean:
	rm -f pagetables $(TESTS)
	rm -f $(OBJS) $(OS_OBJS) $(ARCH_OBJS)
