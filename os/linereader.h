/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __LINEREADER_H__
#define __LINEREADER_H__

#include <string>
#include <fstream>

/*
 * Reader for text lines.
 */

class LineReader
{
  protected:
    std::istream &input;
    bool eofflag;

    LineReader(std::istream &input);

  public:
    virtual ~LineReader()
    {
    }

    /* Methods to get subsequent lines and detect EOF. */
    virtual std::string getLine(void) = 0;
    virtual bool eof(void) const noexcept
    {
      return eofflag;
    }

    /* Factory method for constructing LineReader objects. The input
     * stream is sniffed to detect the file type and the corresponding
     * LineReader object will be constructed.
     */
    static LineReader *create(std::istream &input);
};

#endif /* __LINEREADER_H__ */
