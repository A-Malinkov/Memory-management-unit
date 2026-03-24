/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <fstream>
#include <memory>
#include <list>

#include <cstdint>

class TraceReader;

/*
 * A process performs memory accesses.
 */

enum MemAccessType
{
  Instr,
  Store,
  Load,
  /* A Modify access is done by instructions that modify a memory value
   * in-place. For example "incl (%ecx)".
   */
  Modify
};

struct MemAccess
{
  MemAccessType type;
  uint64_t      addr;
  uint8_t       size;
};

std::ostream &operator<<(std::ostream &stream, const MemAccess &access);

/*
 * Process abstraction.
 */

using PID = uintptr_t;

class Process
{
  private:
    std::unique_ptr<TraceReader> reader;

  public:
    Process(std::istream &input);
    ~Process();

    PID getPID(void) const;

    void getMemoryAccess(MemAccess &access);
    bool finished(void) const;
};

using ProcessList = std::list<std::shared_ptr<Process>>;

#endif /* __PROCESS_H__ */
