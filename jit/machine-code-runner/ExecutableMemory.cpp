#include "ExecutableMemory.hpp"

namespace ovum::vm::jit {

void code_vector::append_uint64(uint64_t value) {
  for (int i = 0; i < 8; i++) {
    push_back(static_cast<uint8_t>(value & 0xFF));
    value >>= 8;
  }
}

ExecutableMemory::ExecutableMemory(size_t size) : size_(size) {
#ifdef _WIN32
  data_ = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
#else
  data_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, 
              MAP_PRIVATE | MAP_ANON, -1, 0);
#endif
}

ExecutableMemory::~ExecutableMemory() {
#ifdef _WIN32
  VirtualFree(data_, 0, MEM_RELEASE);
#else
  munmap(data_, size_);
#endif
}

void* ExecutableMemory::data() const {
  return data_;
}

void ExecutableMemory::make_executable() {
#ifdef _WIN32
  DWORD old;
  VirtualProtect(data_, size_, PAGE_EXECUTE_READ, &old);
#else
  mprotect(data_, size_, PROT_READ | PROT_EXEC);
#endif
}

} // namespace ovum::vm::jit