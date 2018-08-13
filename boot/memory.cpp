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
      usable_mem[i].end_address += end_addr - start_addr;
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

  void memory_map_t::end_mem_add(bootinfo_t &bootinfo)
  {
    bootinfo.tail = (void *)(usable_mem + nr_regions);
  }

  uintptr_t memory_map_t::allocate(size_t size)
  {
    memory_region_t *region = nullptr;
    size_t min_size = 0;
    min_size = ~min_size;

    for(size_t i = 0;i < nr_regions;++i)
    {
      size_t tmp = usable_mem[i].end_address - usable_mem[i].start_address + 1;
      if(tmp < size)
        continue;
      if(tmp > min_size)
        continue;
      region = usable_mem + i;
      min_size = tmp;
    }

    if(!region)
      return 0;
    region->start_address += size;
    return region->start_address - size;
  }
}
