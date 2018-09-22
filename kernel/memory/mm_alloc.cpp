#include <env.hpp>
#include <list.hpp>
#include <init.hpp>
#include <debug.hpp>

#define NO_INLINE_NEW_DELETE
#include <memory.hpp>
#undef NO_INLINE_NEW_DELETE

basic_allocator_t::basic_allocator_t(size_t size)
{
  size = (size + sizeof(size_t) - 1) & ~(sizeof(size) - 1);
  size += sizeof(chunk_entry_t);
  chunk_basic.entry_size = size;

  set_chunk_nr_entries(32); // DEFAULT_CHUNK_NR_ENTRIES
}

bool basic_allocator_t::set_chunk_nr_entries(size_t nr_entries)
{
  if(!nr_entries)
    return false;
  size_t sz1 = sizeof(chunk_header_t);
  size_t sz2 = chunk_basic.entry_size * nr_entries;
  size_t sz3 = ((nr_entries + 63) & ~(size_t)63) >> 3;
  size_t nr_pages = (sz1 + sz2 + sz3 + PAGE_SIZE - 1) >> PAGE_SHIFT;

  return set_chunk_nr_pages(nr_pages);
}

bool basic_allocator_t::set_chunk_nr_pages(size_t nr_pages)
{
  size_t num = nr_pages << PAGE_SHIFT;
  size_t tmp = sizeof(chunk_header_t) - sizeof(uint64_t) - chunk_basic.entry_size;
  if(num < tmp)
    return false;
  num -= tmp;
  num /= sizeof(uint64_t) + (chunk_basic.entry_size << 6);
  ++num;

  chunk_basic.nr_pages = nr_pages;
  chunk_basic.header_size = sizeof(chunk_header_t) + num * sizeof(uint64_t);

  tmp = nr_pages << PAGE_SHIFT;
  tmp -= chunk_basic.header_size;
  chunk_basic.total_entries = tmp / chunk_basic.entry_size;

  return true;
}

void *basic_allocator_t::allocate(void)
{
  if(usable_chunks.empty() && !create_chunk())
    return nullptr;

  chunk_header_t *chunk = usable_chunks.first();
  auto data = (chunk_entry_t *)allocate_from_chunk(*chunk);
  if(!data)
    return nullptr;
  data->header = chunk;

  if(!chunk->free_entries)
    chunk->list_node.remove(),unusable_chunks.insert(*chunk);

  return data + 1;
}

void basic_allocator_t::deallocate(void *_data)
{
  auto data = (chunk_entry_t *)_data;
  --data;

  chunk_header_t *chunk = data->header;
  if(!chunk->free_entries)
    chunk->list_node.remove(),usable_chunks.insert(*chunk);

  deallocate_to_chunk(*chunk,data);
  if(chunk->free_entries == chunk->total_entries)
    destory_chunk(*chunk);
}

bool basic_allocator_t::create_chunk(void)
{
  auto chunk = (chunk_header_t *)alloc_pages(chunk_basic.nr_pages);
  if(!chunk)
    return false;

  *(chunk_basic_t *)chunk = chunk_basic;
  chunk->free_entries = chunk->total_entries;
  chunk->allocator = this;
  usable_chunks.insert(*chunk);

  size_t n = chunk->header_size - sizeof(chunk_header_t);
  n >>= 3;
  for(size_t i = 0;i < n;++i)
    chunk->is_entry_free[i] = ~(uint64_t)0;

  return true;
}

void basic_allocator_t::destory_chunk(chunk_header_t &chunk)
{
  chunk.list_node.remove();
  dealloc_pages((void *)&chunk,chunk.nr_pages);
}

void *basic_allocator_t::allocate_from_chunk(chunk_header_t &chunk)
{
  size_t n = chunk.header_size - sizeof(chunk_header_t);
  n >>= 3;
  for(size_t i = 0;i < n;++i)
  {
    if(!chunk.is_entry_free[i])
      continue;
    --chunk.free_entries;

    uint64_t idx = __builtin_ffsll(chunk.is_entry_free[i]) - 1;
    chunk.is_entry_free[i] -= 1ull << idx;
    idx += i << 6;

    uint8_t *data = (uint8_t *)&chunk;
    data += chunk.header_size;
    data += chunk.entry_size * idx;
    return data;
  }

  return nullptr;
}

void basic_allocator_t::deallocate_to_chunk(chunk_header_t &chunk,void *data)
{
  size_t idx = (uint8_t *)data - (uint8_t *)&chunk;
  idx -= chunk.header_size;
  idx /= chunk.entry_size;

  uint64_t mask = idx & 0x3f;
  mask = 1ull << mask;
  idx >>= 6;

  chunk.is_entry_free[idx] += mask;
  ++chunk.free_entries;
}

namespace mm_alloc
{
  enum : size_t { NR_ALLOCATORS = 14 };

  static constexpr unsigned int allocators_size[NR_ALLOCATORS] =
      {8,16,32,64,128,256,512,1024,1536,2048,2560,3072,3584,4096};
  static uint8_t allocators[sizeof(basic_allocator_t) * NR_ALLOCATORS];

  static inline size_t allocators_index(size_t size)
  {
    if(size <= 8) {
      return 1;
    }else if(size <= 512) {
      return 30 - __builtin_clz((uint32_t)size - 1);
    }else if(size <= 4096) {
      return 14 - ((4096 - size) >> 9);
    }else {
      return 0;
    }
  }

  static inline basic_allocator_t *get_allocator(size_t size)
  {
    if(size_t x = allocators_index(size))
      return (basic_allocator_t *)&allocators[sizeof(basic_allocator_t) * --x];
    return nullptr;
  }

  static int setup_mmalloc(void) INIT_FUNC(kernel,MEM_MMALLOC);

  static int setup_mmalloc(void)
  {
    for(size_t i = 0;i < NR_ALLOCATORS;++i)
    {
      new(allocators + sizeof(basic_allocator_t) * i)
              basic_allocator_t(allocators_size[i]);
    }

    log_t()<<"Initialize the memory management of small memory regions successfully.\n";

    void *test1 = malloc(32);
    void *test2 = malloc(19);
    if(test2)
      free(test2);
    if(test1)
      free(test1);

    return 0;
  }
}

void *malloc(size_t size)
{
  using namespace mm_alloc;

  basic_allocator_t *allocator = get_allocator(size);
  if(!allocator)
    return nullptr;
  return allocator->allocate();
}

void free(void *data)
{
  basic_allocator_t::free(data);
}
