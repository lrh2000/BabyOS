  .section ".text","ax"
  .code64
  .extern _head
  .extern __init_array_start
  .extern __init_array_end
  .extern __rela_start
  .extern __rela_end
  .global exec_init_func
  .global efi_entry
  .global __cxa_pure_virtual

efi_entry:
  movq %rcx,%rsi
  // movq %rdx,%rdx
  call exec_init_func
die:
  cli
0:
  hlt
  jmp 0b

  .align 8
0:.quad 0
9:.quad _head
exec_init_func:
  xorq %r15,%r15
  movq 9b(%rip),%r14
  testq %r14,%r14
  jnz 3f
  leaq _head(%rip),%r15
3:
  movq 0b(%rip),%r14
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

setup_kernel_image:
  leaq stack_top(%rip),%rsp

  // Setup relocations.
  leaq _head(%rip),%rax
  leaq __rela_start(%rip),%rbx
  leaq __rela_end(%rip),%rdx
  cmpq %rbx,%rdx
  je 2f
1:
  movq 8(%rbx),%rcx
  cmpq $8,%rcx // R_X86_64_RELATIVE
  jne 3f
  movq (%rbx),%rcx
  movq 16(%rbx),%r8
  addq %rax,%rcx
  addq %rax,%r8
  movq %r8,(%rcx)
  addq $24,%rbx
  cmpq %rbx,%rdx
  jne 1b
2:

  callq exec_init_func
3:
  jmp die

__cxa_pure_virtual:
  jmp die

  .section ".bss","aw"
  .align 8
  .skip 0x1000
stack_top:

  .section ".init_array.00102","aw"
  .align 8
  .quad setup_kernel_image
