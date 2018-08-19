#include <env.hpp>
#include <boot.hpp>
#include <memory.hpp>
#include <list.hpp>
#include <init.hpp>

namespace mm_pages
{
  static uint64_t *is_page_free;

  static inline void set_page_bit(physaddr_t addr)
  {
    addr >>= PAGE_SHIFT;

    size_t idx = addr >> 6;
    uint64_t mask = addr & 0x3f;
    mask = 1ull << mask;

    is_page_free[idx] |= mask;
  }

  static inline void clear_page_bit(physaddr_t addr)
  {
    addr >>= PAGE_SHIFT;

    size_t idx = addr >> 6;
    uint64_t mask = addr & 0x3f;
    mask = 1ull << mask;

    is_page_free[idx] &= ~mask;
  }

  static inline bool get_page_bit(physaddr_t addr)
  {
    addr >>= PAGE_SHIFT;

    size_t idx = addr >> 6;
    uint64_t mask = addr & 0x3f;
    mask = 1ull << mask;

    return is_page_free[idx] & mask;
  }

  struct free_pages_start_t;

  struct free_pages_end_t
  {
    free_pages_start_t *start;
  };

  struct free_pages_start_t
  {
    size_t nr_pages;
    list_node_t list_node;
    unsigned int list_index;

    free_pages_end_t &get_end(void)
    {
      auto ptr = (uint8_t *)this;
      ptr += nr_pages << PAGE_SHIFT;
      ptr -= sizeof(free_pages_end_t);
      return *(free_pages_end_t *)ptr;
    }

    void dtor(void);
    void ctor(void);
  };

  static list_head_t<free_pages_start_t,&free_pages_start_t::list_node>
                        free_pages[10];

  static inline unsigned int free_pages_list_index(size_t nr_pages)
  {
    unsigned int res = __builtin_clzl((unsigned long)nr_pages);
    res = (sizeof(unsigned long) << 3) - res - 1;
    if(res >= sizeof(free_pages) / sizeof(free_pages[0]))
      return sizeof(free_pages) / sizeof(free_pages[0]) - 1;
    return res;
  }

  void free_pages_start_t::ctor(void)
  {
    list_index = free_pages_list_index(nr_pages);
    free_pages[list_index].insert(*this);

    get_end().start = this;
  }

  void free_pages_start_t::dtor(void)
  {
    list_node.remove();
  }

  static void *alloc_pages_from(free_pages_start_t *x,size_t nr_pages)
  {
    auto phys = addr_virt2phys((virtaddr_t)x);
    clear_page_bit(phys + ((x->nr_pages - nr_pages) << PAGE_SHIFT));
    clear_page_bit(phys + ((nr_pages - 1) << PAGE_SHIFT));

    x->dtor();
    if(x->nr_pages == nr_pages)
      return x;

    x->nr_pages -= nr_pages;
    set_page_bit(phys + ((x->nr_pages - 1) << PAGE_SHIFT));
    x->ctor();
    return (uint8_t *)x + (x->nr_pages << PAGE_SHIFT);
  }

  static void parse_usable_mem(uintptr_t addr,size_t size)
  {
    dealloc_pages((void *)addr_phys2virt((physaddr_t)addr),size >> PAGE_SHIFT);
  }

  static int setup_mmpages(bootinfo_t *bootinfo) INIT_FUNC(kernel,MEM_MMPAGES);

  static int setup_mmpages(bootinfo_t *bootinfo)
  {
    uintptr_t max_addr = bootinfo->memory.get_max_address();
    size_t size = (max_addr + 1) >> (PAGE_SHIFT + 3);
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    is_page_free = (uint64_t *)addr_phys2virt(bootinfo->memory.allocate(size));

    size >>= 3;
    for(size_t i = 0;i < size;++i)
      is_page_free[i] = 0;

    for(size_t i = 0;i < sizeof(free_pages) / sizeof(free_pages[0]);++i)
      new(free_pages + i) typeof(free_pages[i]);

    bootinfo->memory.parse_usable_mem(&parse_usable_mem);

    return 0;
  }
}

void *alloc_pages(size_t nr_pages)
{
  using namespace mm_pages;

  unsigned int idx = free_pages_list_index(nr_pages);

  unsigned int test = 5;
  for(auto x = free_pages[idx].first();x && test;x = free_pages[idx].next(*x),--test)
  {
    if(x->nr_pages < nr_pages)
      continue;
    return alloc_pages_from(x,nr_pages);
  }

  for(++idx;idx < sizeof(free_pages) / sizeof(free_pages[0]);++idx)
    if(auto x = free_pages[idx].first())
      return alloc_pages_from(x,nr_pages);

  // TODO: Retry the original index.
  return nullptr;
}

void dealloc_pages(void *pages,size_t nr_pages)
{
  using namespace mm_pages;

  // TODO: Safe check.
  auto phys = addr_virt2phys((virtaddr_t)pages);
  if(get_page_bit(phys - PAGE_SIZE)) {
    auto end = (free_pages_end_t *)((uint8_t *)pages - sizeof(free_pages_end_t));
    auto start = end->start;
    start->dtor();
    pages = start;
    nr_pages += start->nr_pages;
    phys = addr_virt2phys((virtaddr_t)pages);
  }
  if(get_page_bit(phys + (nr_pages << PAGE_SHIFT))) {
    auto start = (free_pages_start_t *)((uint8_t *)pages + (nr_pages << PAGE_SHIFT));
    nr_pages += start->nr_pages;
    start->dtor();
  }

  set_page_bit(phys);
  set_page_bit(phys + ((nr_pages - 1) << PAGE_SHIFT));

  auto start = (free_pages_start_t *)pages;
  start->nr_pages = nr_pages;
  start->ctor();
}
