/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <functional>

#include "mmu.h"
#include "process.h"

enum InterruptRequest: short
{
  Timer,
  SyscallExit
};

using InterruptHandler = std::function<void(InterruptRequest request)>;

/*
 * Simulated processor. You do not have to modify this class.
 */

class Processor
{
  protected:
    MMU &mmu;
    std::shared_ptr<Process> current;
    InterruptHandler interruptHandler;
    int timerInterval;

  public:
    Processor(MMU &mmu);

    inline MMU &getMMU(void) const noexcept
    {
      return mmu;
    }

    void setProcess(std::shared_ptr<Process> process) noexcept;
    void setTimerInterval(const int interval) noexcept;
    void setInterruptHandler(InterruptHandler fn) noexcept;
    void run(void);
};

#endif /* __PROCESSOR_H__ */
