/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "tracereader.h"
#include "process.h"

const char *type[] = {
  "Instr",
  "Store",
  "Load ",
  "Mod  "
};

std::ostream &
operator<<(std::ostream &stream, const MemAccess &access)
{
  stream << type[access.type] << " ";
  stream << std::hex << std::showbase << access.addr << " "
         << std::dec << std::noshowbase
         << (int)access.size;

  return stream;
}

Process::Process(std::istream &input)
  : reader(std::make_unique<TraceReader>(input))
{
}

Process::~Process()
{
}

uintptr_t
Process::getPID(void) const
{
  /* Use the pointer to the Process as PID */
  return reinterpret_cast<const uintptr_t>(this);
}

void
Process::getMemoryAccess(MemAccess &access)
{
  *reader >> access;
}

bool
Process::finished(void) const
{
  return reader->eof();
}
