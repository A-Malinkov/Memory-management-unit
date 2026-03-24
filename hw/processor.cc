/* pagetables -- A framework to experiment with memory management
 *
 * Copyright (C) 2017--2026 Leiden University, The Netherlands.
 */

#include "processor.h"

#include <iostream>

/*
 * Simulated processor. You do not have to modify this class.
 */

Processor::Processor(MMU &mmu)
  : mmu(mmu), current(nullptr), interruptHandler(nullptr), timerInterval(0)
{
}

/* Set the currently running process to @process.
 */
void
Processor::setProcess(std::shared_ptr<Process> process) noexcept
{
  current = process;
}

/* Set the interval of the timer interrupt to @interval (for this simulation,
 * the interval is measured in memory accesses.)
 */
void
Processor::setTimerInterval(const int interval) noexcept
{
  this->timerInterval = interval;
}

/* Set the timer interrupt handler to @fn.
 */
void
Processor::setInterruptHandler(InterruptHandler fn) noexcept
{
  interruptHandler = fn;
}

/* Execute all processes (memory traces).
 */
void
Processor::run(void)
{
  if(timerInterval == 0)
    throw std::runtime_error("no timer interval (time quantum) set.");

  if(!current){
    /* No current process is active, so generate timer interrupt.
     * (Note that the Exit syscall interrupt can never be triggered in case no
     * process is active.)
     */
    interruptHandler(InterruptRequest::Timer);
  }

  while(current){
    int accesses = 0;

    /* We continue to run this process (trace) until we have reached the end or
     * have reached the timer interval for the timer controller. Because we do
     * not count instructions or time, we approximate the time passed by
     * counting the number of performed memory references.
     */
    while(not current->finished() && accesses < timerInterval){
      MemAccess access;

      current->getMemoryAccess(access);
      mmu.processMemAccess(access);
      accesses++;
    }

    /* Generate interrupt. */
    if(current->finished()){
      /* Process is finished and "has called exit" to request termination */
      interruptHandler(InterruptRequest::SyscallExit);
    }else{
      interruptHandler(InterruptRequest::Timer);
    }
  }
}
