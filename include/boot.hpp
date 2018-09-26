#pragma once
#include <env.hpp>
#include <video.hpp>

struct bootinfo_t;
class console_t;

namespace boot
{
  using pixel_format_t = framebuffer_t::pixel_format_t;

  struct video_early_info_t
  {
    uint32_t height;
    uint32_t width;
    uint32_t vram_width;
    uintptr_t vram_address;
    pixel_format_t pixel_format;
  };

  struct video_info_t
  {
    framebuffer_t *fb;
    console_t *console;
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

    uintptr_t bootmem_addr;
    uint32_t bootmem_size;
    uint32_t bootmem_used_size;

    uintptr_t kernel_addr;
    uint32_t kernel_size;

public:
    size_t begin_mem_add(bootinfo_t &bootinfo);
    void add_usable_mem(uintptr_t start_addr,uintptr_t end_addr,size_t &rest);
    void end_mem_add(bootinfo_t &bootinfo,
            uintptr_t bm_addr,size_t bm_size,uintptr_t kern_addr,size_t kern_size);

    void parse_usable_mem(void (*func)(uintptr_t addr,size_t size));

    inline void update_max_address(uintptr_t addr)
    {
      if(addr > max_address)
        max_address = addr;
    }
    inline uintptr_t get_max_address(void)
    {
      return max_address;
    }

    inline uintptr_t allocate(size_t size)
    {
      if(bootmem_used_size + size < bootmem_used_size)
        return 0;
      if(bootmem_used_size + size > bootmem_size)
        return 0;
      bootmem_used_size += size;
      return bootmem_addr + bootmem_used_size - size;
    }
  };
}

struct bootinfo_t
{
  union {
    boot::video_early_info_t early;
    boot::video_info_t info;
  } video;
  boot::memory_map_t memory;

  uintptr_t acpi_rsdp;

  void *tail;

  enum { MAX_SIZE = 0x1000 };
};
