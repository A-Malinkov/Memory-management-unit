/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include <iostream>
#include <fstream>

#include <getopt.h>

#include "exceptions.h"
#include "settings.h"

#include "simple.h"
#include "riscv.h"

const char *progname;
uint64_t MemorySize = 1 * 1024 * 1024 * 1024; /* Default to 1 GiB */

static void
usage()
{
  std::cerr << "usage: " << progname << " [-l] {-S|-R} ";
  std::cerr << "[-q N] [-m N] [-t N] [-a] filename..." << std::endl;
  std::cerr << R"EOF(
    -l          Log all memory accesses to stderr.

    -S          Simulate "simple" page table.
    -R          Simulate RISC-V Sv48 page table.

    -q quantum  Set process time quantum.
    -m memsize  Set memory size in KiB.

    -t entries  Enable TLB with n entries.
    -a          Enable ASID.

    -H          Enable hole-list based physical memory manager.

One of -S or -R must be specified.
)EOF";
}

template<typename M, typename D> static void
run(uint64_t memorySize, ProcessList &processList)
{
  M mmu;
  D driver;

  Processor processor(mmu);
  OSKernel kernel(processor, driver, memorySize, processList);

  processor.run();
}

static void
error(const char *msg)
{
  std::cerr << "Error: " << msg << std::endl << std::endl;
  usage();
  exit(1);
}

int
main(int argc, char **argv)
{
  progname = argv[0];

  bool useSimple = false;
  bool useRISCV = false;

  /* Parse options */
  for(char c; (c = getopt(argc, argv, "lSRq:m:t:aHh")) != -1;){
    switch (c){
      case 'l':
        LogMemoryAccesses = true;
        break;

      case 'S':
        useSimple = true;
        break;
      case 'R':
        useRISCV = true;
        break;

      case 'q':
        ProcessTimeQuantum = std::stoi(optarg);
        break;
      case 'm':
        MemorySize = std::stoull(optarg) << 10;
        break;

      case 't':
        EnableTLB = true;
        TLBEntries = std::stoi(optarg);
        break;
      case 'a':
        EnableASID = true;
        break;

      case 'H':
        EnableHole = true;
        break;

      case 'h':
      default:
        usage();
        return 1;
    }
  }
  argc -= optind;
  argv += optind;

  if(!useSimple && !useRISCV)
    error("No page table type specified.");
  if((useSimple + useRISCV) > 1)
    error("Can only specify one of {-S|-R}.");

  if(argc == 0)
    error("No input files specified.");

  /* Start main program, catch any exceptions and let the user know. */
  try{
    /* Store opened files in a separate list to keep ownership, pass
     * istream references to Processes.
     */
    std::list<std::ifstream> files;
    ProcessList processList;

    for(int i = 0; i < argc; i++){
      files.emplace_back(std::ifstream(argv[i]));
      if(!files.back())
        throw std::runtime_error("Could not open " + std::string(argv[i]));

      processList.emplace_back(std::make_shared<Process>(files.back()));
    }

    if(useSimple){
      run<Simple::MMU, Simple::MMUDriver>(MemorySize, processList);
    }else if(useRISCV){
      run<RISCV::MMU, RISCV::MMUDriver>(MemorySize, processList);
    }else{
      error("Unknown page table type specified.\n\n");
    }
  }
  catch(TraceFileParseError &error){
    std::cerr << "Parse error: " << error.what() << std::endl;
    return 1;
  }
  catch(ReadError &error){
    std::cerr << "Read error: " << error.what() << std::endl;
    return 1;
  }
  catch(std::runtime_error &error){
    std::cerr << std::endl
        << "===> Runtime error: " << error.what() << std::endl;
    return 1;
  }
  catch(...){
    std::cerr << "Unknown exception occurred." << std::endl;
    return 1;
  }

  return 0;
}
