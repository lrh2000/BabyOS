#include <env.hpp>
#include <init.hpp>
#include <boot.hpp>
#include <debug.hpp>

namespace paging
{
  enum : uint64_t
  {
    ATTR_KERNEL = 0x0000000000000003,
  };

  static uint64_t *kernel_pgd;

  static int setup_paging(bootinfo_t *) INIT_FUNC(kernel,MEM_PAGING);

  static int setup_paging(bootinfo_t *bootinfo)
  {
    auto max_addr = bootinfo->memory.get_max_address();
    if(max_addr < ~(uint32_t)0)
      max_addr = ~(uint32_t)0;

    auto pgd = (uint64_t *)bootinfo->memory.allocate(0x1000);
    if(!pgd)
      return -1;
    for(int pgd_idx = 0;pgd_idx < 512;++pgd_idx)
    {
      size_t pgd_addr = pgd_idx;
      pgd_addr <<= 39;

      if(pgd_addr > max_addr) {
        pgd[pgd_idx] = 0;
        continue;
      }

      pgd[pgd_idx] = bootinfo->memory.allocate(0x1000);
      auto pud = (uint64_t *)pgd[pgd_idx];
      if(!pud)
        return -1;
      pgd[pgd_idx] |= ATTR_KERNEL;
      for(int pud_idx = 0;pud_idx < 512;++pud_idx)
      {
        size_t pud_addr = pud_idx;
        pud_addr <<= 30;
        pud_addr += pgd_addr;

        if(pud_addr > max_addr) {
          pud[pud_idx] = 0;
          continue;
        }

        pud[pud_idx] = bootinfo->memory.allocate(0x1000);
        auto pmd = (uint64_t *)pud[pud_idx];
        if(!pmd)
          return -1;
        pud[pud_idx] |= ATTR_KERNEL;
        for(int pmd_idx = 0;pmd_idx < 512;++pmd_idx)
        {
          size_t pmd_addr = pmd_idx;
          pmd_addr <<= 21;
          pmd_addr += pud_addr;

          if(pmd_addr > max_addr) {
            pmd[pmd_idx] = 0;
            continue;
          }

          pmd[pmd_idx] = bootinfo->memory.allocate(0x1000);
          auto pte = (uint64_t *)pmd[pmd_idx];
          if(!pte)
            return -1;
          pmd[pmd_idx] |= ATTR_KERNEL;
          for(int pte_idx = 0;pte_idx < 512;++pte_idx)
          {
            size_t addr = pte_idx;
            addr <<= 12;
            addr += pmd_addr;

            if(addr > max_addr)
              pte[pte_idx] = 0;
            else
              pte[pte_idx] = addr | ATTR_KERNEL;
          }
        }
      }
    }

    kernel_pgd = pgd;
    asm volatile("movq %0,%%cr3" : : "r"(pgd) : "memory");

    log_t()<<"Initialize the kernel page tables successfully.\n";

    return 0;
  }
}
