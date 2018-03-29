#ifndef ONEFLOW_CORE_COMMON_BLAS_H_
#define ONEFLOW_CORE_COMMON_BLAS_H_

#include <type_traits>
#include <utility>
#include "oneflow/core/common/cblas.h"
#include "oneflow/core/common/preprocessor.h"

namespace oneflow {

#define BLAS_NAME_SEQ        \
  OF_PP_MAKE_TUPLE_SEQ(dot)  \
  OF_PP_MAKE_TUPLE_SEQ(swap) \
  OF_PP_MAKE_TUPLE_SEQ(copy) \
  OF_PP_MAKE_TUPLE_SEQ(axpy) \
  OF_PP_MAKE_TUPLE_SEQ(scal) \
  OF_PP_MAKE_TUPLE_SEQ(gemv) \
  OF_PP_MAKE_TUPLE_SEQ(gemm)

#define CBLAS_TEMPLATE(name)                                               \
  template<typename T, typename... Args>                                   \
  auto cblas_##name(Args&&... args)                                        \
      ->typename std::enable_if<std::is_same<T, float>::value,             \
                                decltype(cblas_##s##name(                  \
                                    std::forward<Args>(args)...))>::type { \
    return cblas_##s##name(std::forward<Args>(args)...);                   \
  }                                                                        \
  template<typename T, typename... Args>                                   \
  auto cblas_##name(Args&&... args)                                        \
      ->typename std::enable_if<std::is_same<T, double>::value,            \
                                decltype(cblas_##d##name(                  \
                                    std::forward<Args>(args)...))>::type { \
    return cblas_##d##name(std::forward<Args>(args)...);                   \
  }

OF_PP_FOR_EACH_TUPLE(CBLAS_TEMPLATE, BLAS_NAME_SEQ);

#undef CBLAS_TEMPLATE

#ifdef WITH_CUDA

#define CUBLAS_TEMPLATE(name)                                                  \
  template<typename T, typename... Args>                                       \
  typename std::enable_if<std::is_same<T, float>::value>::type cublas_##name(  \
      Args&&... args) {                                                        \
    CudaCheck(cublasS##name(std::forward<Args>(args)...));                     \
  }                                                                            \
  template<typename T, typename... Args>                                       \
  typename std::enable_if<std::is_same<T, double>::value>::type cublas_##name( \
      Args&&... args) {                                                        \
    CudaCheck(cublasD##name(std::forward<Args>(args)...));                     \
  }

OF_PP_FOR_EACH_TUPLE(CUBLAS_TEMPLATE, BLAS_NAME_SEQ);

#endif  // WITH_CUDA

#undef BLAS_NAME_SEQ

}  // namespace oneflow

#endif  // ONEFLOW_CORE_COMMON_BLAS_H_