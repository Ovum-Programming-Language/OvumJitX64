#ifndef JIT_EXECUTABLEMEMORY_HPP
#define JIT_EXECUTABLEMEMORY_HPP

#include <cstdint>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

namespace ovum::vm::jit {

class code_vector : public std::vector<uint8_t> {
public:
  void append_uint64(uint64_t value);
};

class ExecutableMemory {
  void* data_;
  size_t size_;

public:
  ExecutableMemory(size_t size);

  ~ExecutableMemory();

  void* data() const;

  void make_executable();
};

} // namespace ovum::vm::jit

#endif // JIT_EXECUTABLEMEMORY_HPP
