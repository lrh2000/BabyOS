  .section ".data","aw"
  .global irq_entry_table
irq_entry_table:

  .section ".text","ax"
  .extern do_interrupt

#define IRQ_ENTRY(i) \
  .section ".data","aw"; \
  .quad irq_entry_##i; \
  .section ".text","ax"; \
irq_entry_##i##:; \
  pushq $i; \
  jmp irq_entry

  IRQ_ENTRY(0)
  IRQ_ENTRY(1)
  IRQ_ENTRY(2)
  IRQ_ENTRY(3)
  IRQ_ENTRY(4)
  IRQ_ENTRY(5)
  IRQ_ENTRY(6)
  IRQ_ENTRY(7)
  IRQ_ENTRY(8)
  IRQ_ENTRY(9)
  IRQ_ENTRY(10)
  IRQ_ENTRY(11)
  IRQ_ENTRY(12)
  IRQ_ENTRY(13)
  IRQ_ENTRY(14)
  IRQ_ENTRY(15)
  IRQ_ENTRY(16)
  IRQ_ENTRY(17)
  IRQ_ENTRY(18)
  IRQ_ENTRY(19)
  IRQ_ENTRY(20)
  IRQ_ENTRY(21)
  IRQ_ENTRY(22)
  IRQ_ENTRY(23)
  IRQ_ENTRY(24)

irq_entry:
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

  movq 120(%rsp),%rdi
  callq do_interrupt

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
  addq $8,%rsp
  iretq
