#pragma once
#include <env.hpp>

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

  struct memory_map_t
  {
    size_t nr_regions;
    memory_region_t *usable_mem;
  };
}

struct bootinfo_t
{
  boot::video_info_t video;
  boot::memory_map_t memory;

  void *tail;

  enum { MAX_SIZE = 0x1000 };
};
