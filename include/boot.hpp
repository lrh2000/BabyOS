#pragma once
#include <env.hpp>

struct bootinfo_t;

namespace boot
{
  struct video_info_t
  {
    uint32_t height;
    uint32_t width;
    uint32_t vram_width;
    //framebuffer_t::pixel_format_t pixel_format;
    uintptr_t vram_address;
  };

  struct memory_region_t
  {
    uintptr_t start_address;
    uintptr_t end_address;
  };

  class memory_map_t
  {
    size_t nr_regions;
    memory_region_t *usable_mem;
    uintptr_t max_address;

public:
    size_t begin_mem_add(bootinfo_t &bootinfo);
    void add_usable_mem(uintptr_t start_addr,uintptr_t end_addr,size_t &rest);
    void end_mem_add(bootinfo_t &bootinfo);

    inline void update_max_address(uintptr_t addr)
    {
      if(addr > max_address)
        max_address = addr;
    }
    inline uintptr_t get_max_address(void)
    {
      return max_address;
    }

    uintptr_t allocate(size_t size);
  };
}

struct bootinfo_t
{
  boot::video_info_t video;
  boot::memory_map_t memory;

  void *tail;

  enum { MAX_SIZE = 0x1000 };
};
