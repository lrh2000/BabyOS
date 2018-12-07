#pragma once
#include <env.hpp>

namespace init_order
{
  namespace boot
  {
    enum { EFI          = 101 };
    enum { KERNEL_IMAGE = 102 };
  }

  namespace kernel
  {
    enum { CPU_GDT     = 1000 };
    enum { MEM_PAGING  = 1001 };
    enum { MEM_MMPAGES = 1002 };
    enum { MEM_MMALLOC = 1003 };
    enum { VIDEO_FB    = 1004 };
    enum { VIDEO_CON   = 1005 };
    enum { DEBUG_LOG   = 1006 };

    enum { CPU_IDT     = 1007 };
    enum { ACPI_ACPI   = 1008 };
    enum { IRQ_IOAPIC  = 1009 };
    enum { IRQ_LAPIC   = 1010 };
    enum { TIME_GTIMER = 1011 };
    enum { TIME_LTIMER = 1012 };
    enum { TASK_TASK   = 1013 };
  }

  namespace fs
  {
  }

  namespace driver
  {
  }
}

#define INIT_FUNC(module,func) \
  __attribute__((constructor(::init_order::module::func)))
