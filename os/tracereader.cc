/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "tracereader.h"
#include "exceptions.h"

TraceReader::TraceReader(std::istream &input)
  : lineReader(LineReader::create(input)), eofflag(false),
    lineno(0), currentAccess()
{
  parseToNextAccess();
}

/* Parse the type of a single trace line @str, and return it in @access.
 */
void
TraceReader::parseType(const std::string &str, MemAccess &access)
{
  if(str[0] == 'I'){
    access.type = MemAccessType::Instr;
    return;
  }
  switch(str[1]){
    case 'S':
      access.type = MemAccessType::Store;
      break;
    case 'L':
      access.type = MemAccessType::Load;
      break;
    case 'M':
      access.type = MemAccessType::Modify;
      break;
    default:
      throw TraceFileParseError(lineno, "invalid memory access type '" +
                                str.substr(0, 2) + "'");
  }
}

/* Parse a single trace line @line, and return it in @access.
 */
bool
TraceReader::parseLine(const std::string &line, MemAccess &access)
{
  if(line.substr(0, 2) == "==")
    return false;

  /* A line containing a memory access must be at least 6 characters
   * long "IL ", followed by an address, comma and size all at least
   * one character in size.
   */
  if(line.size() < 6)
    return false;

  parseType(line, access);

  const size_t pos = line.find(',', 3);
  if(pos == 3 or pos == line.size() - 1)
    throw TraceFileParseError(lineno, "malformed address/size pair");

  try{
    access.addr = std::stoull(line.substr(3, pos - 3), nullptr, 16);
    access.size = std::stoul(line.substr(pos + 1));
  }
  catch(...){
    throw TraceFileParseError(lineno, "malformed address or size");
  }
  return true;
}

/* Parse the current trace up to the next memory access.
 */
void
TraceReader::parseToNextAccess()
{
  while(not lineReader->eof()){
    std::string line(lineReader->getLine());
    lineno++;

    if(parseLine(line, currentAccess))
      return;
  }
  eofflag = true;
}

TraceReader &
TraceReader::operator>>(MemAccess &access)
{
  access = currentAccess;
  parseToNextAccess();
  return *this;
}
