// Copyright © 2023 Apple Inc.

#include <string>

#ifdef MLX_HAS_DLOPEN_NOLOAD
#include <dlfcn.h>
#endif

#ifdef MLX_HAS_DL_ITERATE_PHDR
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <link.h>
#endif

#ifdef MLX_HAS_DYLD_GET_IMAGE_NAME
#include <mach-o/dyld.h>
#endif

#include "mlx/data/core/ThreadController.h"

namespace {
#ifdef MLX_HAS_DL_ITERATE_PHDR
int dl_iterate_callback(struct dl_phdr_info* info, size_t size, void* data) {
  auto libNames = reinterpret_cast<std::vector<std::string>*>(data);
  libNames->push_back(info->dlpi_name);
  return 0;
}
std::vector<std::string> find_loaded_lib_names() {
  std::vector<std::string> libNames;
  dl_iterate_phdr(dl_iterate_callback, &libNames);
  return libNames;
}
#elif defined(MLX_HAS_DYLD_GET_IMAGE_NAME)
std::vector<std::string> find_loaded_lib_names() {
  std::vector<std::string> lib_names;
  auto num_libs = _dyld_image_count();
  for (auto i = 0; i < num_libs; i++) {
    lib_names.push_back(_dyld_get_image_name(i));
  }
  return lib_names;
}
#else
std::vector<std::string> find_loaded_lib_names() {
  return std::vector<std::string>();
}
#endif
} // namespace

namespace mlx {
namespace data {
namespace core {

struct ThreadControllerSym {
  ThreadControllerSym(void* lib, void* get_sym, void* set_sym)
      : lib(lib), get_sym(get_sym), set_sym(set_sym) {};
  void* lib;
  void* get_sym;
  void* set_sym;
#ifdef MLX_HAS_DLOPEN_NOLOAD
  ~ThreadControllerSym() {
    if (lib) {
      dlclose(lib);
    }
  }
#endif
};

ThreadController::ThreadController() {
  std::vector<std::string> get_names = {
      "MKL_Get_Max_Threads",
      "openblas_get_num_threads",
      "openblas_get_num_threads64_",
      "omp_get_num_threads"};
  std::vector<std::string> set_names = {
      "MKL_Set_Num_Threads",
      "openblas_set_num_threads",
      "openblas_set_num_threads64_",
      "omp_set_num_threads"};

#ifdef MLX_HAS_DLOPEN_NOLOAD
  auto lib_names = find_loaded_lib_names();
  for (auto i = 0; i < get_names.size(); i++) {
    auto get_name = get_names[i];
    auto set_name = set_names[i];
    for (auto l = 0; l <= lib_names.size(); l++) {
      // at the last resort, we test main executable
      const char* lib_name =
          (l < lib_names.size() ? lib_names[l].c_str() : nullptr);
      auto lib = dlopen(lib_name, RTLD_NOW | RTLD_NOLOAD);
      if (lib) {
        auto get_sym = dlsym(lib, get_name.c_str());
        auto set_sym = dlsym(lib, set_name.c_str());
        if (get_sym && set_sym) {
          symbols_.push_back(
              std::make_shared<ThreadControllerSym>(lib, get_sym, set_sym));
          break;
        }
      } else {
        dlclose(lib);
      }
    }
  }
#endif
}

ThreadControllerState ThreadController::limit() {
  ThreadControllerState state(symbols_.size());
  for (auto i = 0; i < symbols_.size(); i++) {
    auto symbol = symbols_[i];
    state[i] = reinterpret_cast<int (*)(void)>(symbol->get_sym)();
    reinterpret_cast<void (*)(int)>(symbol->set_sym)(1);
  }
  return state;
}

void ThreadController::restore(const ThreadControllerState& state) {
  for (auto i = 0; i < symbols_.size(); i++) {
    auto symbol = symbols_[i];
    reinterpret_cast<void (*)(int)>(symbol->set_sym)(state[i]);
  }
}
} // namespace core
} // namespace data
} // namespace mlx
