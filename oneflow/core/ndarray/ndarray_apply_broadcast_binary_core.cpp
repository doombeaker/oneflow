#include "oneflow/core/ndarray/ndarray_apply_broadcast_binary_core.h"

namespace oneflow {

template<typename T, int NDIMS, const T (*binary_func)(const T, const T)>
struct NdarrayApplyBroadcastBinaryCoreWrapper<DeviceType::kCPU, T, NDIMS, binary_func> final {
  static void Apply(DeviceCtx* ctx, const XpuVarNdarray<T>& y, const XpuVarNdarray<const T>& a,
                    const XpuVarNdarray<const T>& b) {
    NdarrayApplyBroadcastBinaryCore<T, NDIMS, binary_func>::Apply(y, a, b);
  }
  static void ImplaceApply(DeviceCtx* ctx, const XpuVarNdarray<T>& y,
                           const XpuVarNdarray<const T>& x) {
    NdarrayApplyBroadcastBinaryCore<T, NDIMS, binary_func>::ImplaceApply(y, x);
  }
};

#define INSTANTIATE_BROADCAST_BINARY_FUNC(dtype_pair, NDIMS, binary_func) \
  template struct NdarrayApplyBroadcastBinaryCoreWrapper<                 \
      DeviceType::kCPU, OF_PP_PAIR_FIRST(dtype_pair), NDIMS, binary_func>;
OF_PP_SEQ_PRODUCT_FOR_EACH_TUPLE(INSTANTIATE_BROADCAST_BINARY_FUNC, ARITHMETIC_DATA_TYPE_SEQ,
                                 DIM_SEQ, ARITHMETIC_BINARY_FUNC_SEQ)
}  // namespace oneflow