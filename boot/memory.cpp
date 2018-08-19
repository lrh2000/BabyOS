#include <boot.hpp>

namespace boot
{
  size_t memory_map_t::begin_mem_add(bootinfo_t &bootinfo)
  {
    max_address = 0;
    nr_regions = 0;
    usable_mem = (memory_region_t *)bootinfo.tail;

    size_t rest = (uint8_t *)bootinfo.tail - (uint8_t *)&bootinfo;
    rest = bootinfo_t::MAX_SIZE - rest;
    rest /= sizeof(usable_mem[0]);
    return rest;
  }

  void memory_map_t::add_usable_mem(uintptr_t start_addr,uintptr_t end_addr,size_t &rest)
  {
    size_t i;

    if(start_addr == 0)
      start_addr = 0x1000;
    if(end_addr < start_addr)
      return;

    for(i = nr_regions - 1;~i;--i)
    {
      if(usable_mem[i].end_address + 1 < start_addr)
        break;
      if(usable_mem[i].end_address + 1 > start_addr)
        continue;
      usable_mem[i].end_address += end_addr - start_addr + 1;
      return;
    }

    if(!rest--)
      return;

    for(i = nr_regions - 1;~i;--i)
    {
      if(usable_mem[i].end_address + 1 < start_addr)
        break;
      usable_mem[i + 1] = usable_mem[i];
    }
    ++i;
    ++nr_regions;

    usable_mem[i].start_address = start_addr;
    usable_mem[i].end_address = end_addr;
  }

  void memory_map_t::end_mem_add(bootinfo_t &bootinfo,
            uintptr_t bm_addr,size_t bm_size,uintptr_t kern_addr,size_t kern_size)
  {
    bootinfo.tail = (void *)(usable_mem + nr_regions);

    bootmem_addr = bm_addr;
    if(bm_size > ~(uint32_t)0)
      bootmem_size = ~(uint32_t)0;
    else
      bootmem_size = bm_size;
    bootmem_used_size = 0;

    kernel_addr = kern_addr;
    kernel_size = kern_size;
  }

  void memory_map_t::parse_usable_mem(void (*func)(uintptr_t addr,size_t size))
  {
    uintptr_t now = 0;
    size_t i = 0;

    while(i < nr_regions)
    {
      uintptr_t start = usable_mem[i].start_address;
      uintptr_t end = usable_mem[i].end_address;

      if(now > start)
        start = now;
      if(start > end) {
        ++i;
        continue;
      }
      now = end + 1;

      if(kernel_addr < end && kernel_addr >= start)
        end = kernel_addr - 1,now = kernel_addr + kernel_size;
      if(bootmem_addr < end && bootmem_addr >= start)
        end = bootmem_addr - 1,now = bootmem_addr + bootmem_used_size;
      if(start > end)
        continue;

      (*func)(start,end - start + 1);
    }
  }
}
