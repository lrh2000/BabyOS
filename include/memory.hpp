#pragma once
#include <list.hpp>

typedef uintptr_t physaddr_t;
typedef uintptr_t virtaddr_t;

enum : size_t { PAGE_SIZE   = 0x1000 };
enum : size_t { PAGE_SHIFT  = 12     };
enum : size_t { PAGE_OFFSET = 0      };

inline physaddr_t addr_virt2phys(virtaddr_t addr)
{
  return addr - PAGE_OFFSET;
}

inline virtaddr_t addr_phys2virt(physaddr_t addr)
{
  return addr + PAGE_OFFSET;
}

void *alloc_pages(size_t nr_pages);
void dealloc_pages(void *pages,size_t nr_pages);

void *malloc(size_t size);
void free(void *data);

#ifdef NO_INLINE_NEW_DELETE
#define inline /* Nothing here. */
#endif

inline void *operator new(size_t,void *data)
{
  return data;
}
inline void *operator new(size_t size)
{
  return malloc(size);
}

inline void operator delete(void *data,size_t)
{
  free(data);
}
inline void operator delete(void *data)
{
  free(data);
}
inline void operator delete(void *,void *)
{
  // Nothing to do.
}

#ifdef NO_INLINE_NEW_DELETE
#undef inline
#endif

class basic_allocator_t
{
  struct chunk_basic_t
  {
    size_t nr_pages;
    size_t header_size;

    size_t entry_size;
    size_t total_entries;
  };

  struct chunk_header_t : public chunk_basic_t
  {
    size_t free_entries;

    list_node_t list_node;
    basic_allocator_t *allocator;

    uint64_t is_entry_free[0];
  };

  struct chunk_entry_t
  {
    chunk_header_t *header;
  };

  list_head_t<chunk_header_t,&chunk_header_t::list_node>
              usable_chunks;
  list_head_t<chunk_header_t,&chunk_header_t::list_node>
              unusable_chunks;

  chunk_basic_t chunk_basic;

public:
  basic_allocator_t(size_t size);

  bool set_chunk_nr_pages(size_t nr_pages);
  bool set_chunk_nr_entries(size_t nr_entries);

  void *allocate(void);
  void deallocate(void *data);

  static inline void free(void *_data)
  {
    auto data = (chunk_entry_t *)_data;
    --data;
    data->header->allocator->deallocate(_data);
  }

protected:
  bool create_chunk(void);
  void destory_chunk(chunk_header_t &chunk);

  void *allocate_from_chunk(chunk_header_t &chunk);
  void deallocate_to_chunk(chunk_header_t &chunk,void *data);
};

template<typename T>
  class allocator_t
{
  basic_allocator_t basic;
public:
  inline allocator_t(void) : basic(sizeof(T)) {}

  inline T *allocate(void)
  {
    return (T *)basic.allocate();
  }
  inline void deallocate(T *data)
  {
    basic.deallocate((void *)data);
  }

  static inline void free(T *data)
  {
    basic_allocator_t::free((void *)data);
  }
};
