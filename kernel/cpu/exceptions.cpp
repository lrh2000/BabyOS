#include <intr.hpp>
#include <debug.hpp>
#include "idt.hpp"

using exception_stack_t = cpu::exception_stack_t;

namespace exceptions
{
  typedef bool (*handler_t)(exception_stack_t *);
  enum { NO_INTR_START = 0,NO_INTR_END = 31 };
  enum { NR_EXCEPTIONS = NO_INTR_END - NO_INTR_START + 1};

  // extern bool page_fault(exception_stack_t *);

  bool default_handler(exception_stack_t *)
  {
    return false;
  }

  static handler_t handlers[NR_EXCEPTIONS] = {
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
    &default_handler,
  };

  static constexpr const char *name[NR_EXCEPTIONS] = {
    "Divide Error Exception (#DE)",
    "Debug Exception (#DB)",
    "NMI Interrupt",
    "Breakpoint Exception (#BP)",
    "Overflow Exception (#OF)",
    "BOUND Range Exceeded Exception (#BR)",
    "Invaild Opcode Exception (#UD)",
    "Device Not Available Exception (#NM)",
    "Double Fault Exception (#DF)",
    "Coprocessor Segment Overrun",
    "Invaild TSS Exception (#TS)",
    "Segment Not Present (#NP)",
    "Stack Fault Exception (#SS)",
    "General Protection Exception (#GP)",
    "Page-Fault Exception (#PF)",
    "Unknown Exception",
    "x87 FPU Floating-Point Error (#MF)",
    "Alignment Check Exception (#AC)",
    "Machine-Check Exception (#MC)",
    "SIMD Floating-Point Exception (#XM)",
    "Virtualization Exception (#VE)",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
    "Unknown Exception",
  };
}

extern "C" void do_exception(exception_stack_t *data)
{
  uint64_t idx = data->no_excep - exceptions::NO_INTR_START;
  exceptions::handler_t handler = exceptions::handlers[idx];
  if((*handler)(data))
    return;

  log_t(log_t::NONE)<<"\n";
  log_t(log_t::FATAL)<<"Unexpected Exception!!!\n";
  log_t(log_t::FATAL)<<"Expection "<<idx<<" --- "<<exceptions::name[idx]<<"\n";
  log_t(log_t::FATAL)<<log_t::setbase(16)<<log_t::setw(16)
    <<"RAX:0x"<<data->rax<<" RBX:0x"<<data->rbx<<" RCX:0x"<<data->rcx<<" RDX:0x"<<data->rdx<<"\n";
  log_t(log_t::FATAL)<<log_t::setbase(16)<<log_t::setw(16)
    <<"RSI:0x"<<data->rsi<<" RDI:0x"<<data->rdi<<" RSP:0x"<<data->rsp<<" RBP:0x"<<data->rbp<<"\n";
  log_t(log_t::FATAL)<<log_t::setbase(16)<<log_t::setw(16)
    <<" R8:0x"<<data->r8<<"  R9:0x"<<data->r9<<" R10:0x"<<data->r10<<" R11:0x"<<data->r11<<"\n";
  log_t(log_t::FATAL)<<log_t::setbase(16)<<log_t::setw(16)
    <<"R12:0x"<<data->r12<<" R13:0x"<<data->r13<<" R14:0x"<<data->r14<<" R15:0x"<<data->r15<<"\n";
  log_t(log_t::FATAL)<<log_t::setbase(16)<<log_t::setw(16)
    <<"RIP:0x"<<data->rip<<" RFLAGS:0x"<<data->rflags<<" Error Code:0x"<<data->err_code<<"\n";
  log_t(log_t::FATAL)<<"The expection cann't be handled,which panics the kernel. Stop here.\n";

  asm volatile("cli;0:hlt;jmp 0b");

  __builtin_unreachable();
}
