#ifndef ONEFLOW_CORE_NDARRAY_NDARRAY_APPLY_BROADCAST_BINARY_CORE_H_
#define ONEFLOW_CORE_NDARRAY_NDARRAY_APPLY_BROADCAST_BINARY_CORE_H_

#include "oneflow/core/ndarray/xpu_util.h"
#include "oneflow/core/ndarray/xpu_var_ndarray.h"
#include "oneflow/core/ndarray/xpu_broadcast_ndarray.h"
#include "oneflow/core/ndarray/xpu_binary_func_ndarray.h"
#include "oneflow/core/ndarray/binary_func.h"
#include "oneflow/core/kernel/kernel_util.h"

namespace oneflow {

template<DeviceType device_type, typename T, int NDIMS, const T (*binary_func)(const T, const T)>
struct NdarrayApplyBroadcastBinaryCoreWrapper final {
  static void Apply(DeviceCtx* ctx, const XpuVarNdarray<T>& y, const XpuVarNdarray<const T>& a,
                    const XpuVarNdarray<const T>& b);
  static void ImplaceApply(DeviceCtx* ctx, const XpuVarNdarray<T>& y,
                           const XpuVarNdarray<const T>& x);
};

template<typename T, int NDIMS, const T (*binary_func)(const T, const T)>
struct NdarrayApplyBroadcastBinaryCore final {
  OF_DEVICE_FUNC static void Apply(const XpuVarNdarray<T>& y, const XpuVarNdarray<const T>& a,
                                   const XpuVarNdarray<const T>& b) {
    const auto& ret =
        a.Broadcast(y.shape()).template BinaryFunc<binary_func>(b.Broadcast(y.shape()));
    y.template Assign<NDIMS>(ret);
  }
  OF_DEVICE_FUNC static void ImplaceApply(const XpuVarNdarray<T>& y,
                                          const XpuVarNdarray<const T>& x) {
    y.template BinaryAssign<binary_func, NDIMS>(x.Broadcast(y.shape()));
  }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_NDARRAY_NDARRAY_APPLY_BROADCAST_BINARY_CORE_H_