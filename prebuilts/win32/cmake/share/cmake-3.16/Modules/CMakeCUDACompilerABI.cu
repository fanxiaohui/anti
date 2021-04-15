#ifndef __CUDACC__
#  error "A C or C++ compiler has been selected for CUDA"
#endif

#include "CMakeCompilerABI.h"

int main(int argc, char* argv[])
{
  int require = 0;
  require += info_sizeof_dptr[argc];
#if defined(ABI_ID)
  require += info_abi[argc];
#endif
  (void)argv;
  return require;
}
