  .section ".text","ax"
  .extern do_exception

#define EXCEPTION_ENTRY_EC(i) \
  .global exception_entry_##i; \
exception_entry_##i##:; \
  pushq $i; \
  jmp exception_entry

#define EXCEPTION_ENTRY(i) \
  .global exception_entry_##i; \
exception_entry_##i##:; \
  pushq $~0; \
  pushq $i; \
  jmp exception_entry

  EXCEPTION_ENTRY(0)
  EXCEPTION_ENTRY(1)
  EXCEPTION_ENTRY(2)
  EXCEPTION_ENTRY(3)
  EXCEPTION_ENTRY(4)
  EXCEPTION_ENTRY(5)
  EXCEPTION_ENTRY(6)
  EXCEPTION_ENTRY(7)
  EXCEPTION_ENTRY_EC(8)
  EXCEPTION_ENTRY(9)
  EXCEPTION_ENTRY_EC(10)
  EXCEPTION_ENTRY_EC(11)
  EXCEPTION_ENTRY_EC(12)
  EXCEPTION_ENTRY_EC(13)
  EXCEPTION_ENTRY_EC(14)
  EXCEPTION_ENTRY(15)
  EXCEPTION_ENTRY(16)
  EXCEPTION_ENTRY_EC(17)
  EXCEPTION_ENTRY(18)
  EXCEPTION_ENTRY(19)
  EXCEPTION_ENTRY(20)
  EXCEPTION_ENTRY(21)
  EXCEPTION_ENTRY(22)
  EXCEPTION_ENTRY(23)
  EXCEPTION_ENTRY(24)
  EXCEPTION_ENTRY(25)
  EXCEPTION_ENTRY(26)
  EXCEPTION_ENTRY(27)
  EXCEPTION_ENTRY(28)
  EXCEPTION_ENTRY(29)
  EXCEPTION_ENTRY(30)
  EXCEPTION_ENTRY(31)

exception_entry:
  pushq %rbp
  pushq %rax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rsi
  pushq %rdi
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15

  leaq (%rsp),%rdi
  callq do_exception

  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rdi
  popq %rsi
  popq %rdx
  popq %rcx
  popq %rbx
  popq %rax
  popq %rbp
  addq $16,%rsp
  iretq
