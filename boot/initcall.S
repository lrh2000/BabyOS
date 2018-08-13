  .section ".text","ax"
  .code64
  .extern _head
  .extern __init_array_start
  .extern __init_array_end
  .global exec_init_func
  .global efi_entry

efi_entry:
  movq %rcx,%rsi
  // movq %rdx,%rdx
  call exec_init_func
die:
  cli
0:
  hlt
  jmp 0b

setup_stack:
  leaq stack_top(%rip),%rsp
  call exec_init_func
  jmp die

  .align 8
0:.quad 0
exec_init_func:
  leaq 0b(%rip),%r15
  movq 0b(%rip),%r14
  addq $(_head - 0b),%r15
  leaq __init_array_start(%rip),%rbx
  leaq (%rbx,%r14,8),%rbx
  leaq __init_array_end(%rip),%r13
  cmpq %r13,%rbx
  je 2f
1:
  incq 0b(%rip)
  movq (%rbx),%rax
  addq %r15,%rax
  leaq _head(%rip),%rdi
  call *%rax
  testq %rax,%rax
  jnz 2f
  addq $8,%rbx
  cmpq %r13,%rbx
  jne 1b
2:
  retq

  .section ".bss","aw"
  .align 8
  .skip 0x1000
stack_top:

  .section ".init_array.00102","aw"
  .align 8
  .quad setup_stack
