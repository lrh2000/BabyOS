#pragma once

typedef uintptr_t physaddr_t;
typedef uintptr_t virtaddr_t;

enum : size_t { PAGE_SIZE   = 0x1000 };
enum : size_t { PAGE_SHIFT  = 12     };
enum : size_t { PAGE_OFFSET = 0      };

static inline physaddr_t addr_virt2phys(virtaddr_t addr)
{
  return addr - PAGE_OFFSET;
}

static inline virtaddr_t addr_phys2virt(physaddr_t addr)
{
  return addr + PAGE_OFFSET;
}

inline void *operator new(size_t,void *p)
{
  return p;
}

void *alloc_pages(size_t nr_pages);
void dealloc_pages(void *pages,size_t nr_pages);
