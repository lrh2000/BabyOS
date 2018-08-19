#pragma once

class list_node_t
{
protected:
  list_node_t *_prev,*_next;

public:
  inline void insert_before(list_node_t &node)
  {
    this->_next = &node;
    this->_prev = node._prev;
    this->_prev->_next = this;
    this->_next->_prev = this;
  }
  inline void insert_after(list_node_t &node)
  {
    this->_next = node._next;
    this->_prev = &node;
    this->_prev->_next = this;
    this->_next->_prev = this;
  }

  inline void remove(void)
  {
    this->_prev->_next = this->_next;
    this->_next->_prev = this->_prev;
  }

  template<typename T,list_node_t T::*T_OFFSET>
    friend class list_head_t;
};

template<typename T,list_node_t T::*T_OFFSET>
  class list_head_t : protected list_node_t
{
  constexpr static inline uintptr_t T_OFFSET_INT(void)
  {
    return (uintptr_t)&(((T *)nullptr)->*T_OFFSET);
  }

public:
  inline list_head_t(void)
  {
    _prev = _next = this;
  }

  inline void insert(T &data)
  {
    (data.*T_OFFSET).insert_after(*this);
  }

  inline T *next(T &data)
  {
    list_node_t *node = (data.*T_OFFSET)._next;
    if(node == this)
      return nullptr;
    return (T *)((uintptr_t)node - T_OFFSET_INT());
  }
  inline T *prev(T &data)
  {
    list_node_t *node = (data.*T_OFFSET)._prev;
    if(node == this)
      return nullptr;
    return (T *)((uintptr_t)node - T_OFFSET_INT());
  }

  inline T *first(void)
  {
    list_node_t *node = this->_next;
    if(node == this)
      return nullptr;
    return (T *)((uintptr_t)node - T_OFFSET_INT());
  }
  inline T *last(void)
  {
    list_node_t *node = this->_prev;
    if(node == this)
      return nullptr;
    return (T *)((uintptr_t)node - T_OFFSET_INT());
  }
};
