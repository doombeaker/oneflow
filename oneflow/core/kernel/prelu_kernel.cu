#include "oneflow/core/device/cuda_util.h"
#include "oneflow/core/kernel/kernel_util.h"
#include "oneflow/core/kernel/prelu_kernel.h"
#include "oneflow/core/kernel/kernel_util.cuh"
#include "oneflow/core/device/cuda_util.h"
#include <cub/cub.cuh>

namespace oneflow {
namespace {
template<typename T>
__global__ void PReluForward(const int64_t elem_cnt, const int64_t channel_num, const int64_t area,
                             const T* in_dptr, const T* alpha_dptr, T* out_dptr) {
  CUDA_1D_KERNEL_LOOP(i, elem_cnt) {
    int64_t c = (i / area) % channel_num;
    out_dptr[i] = (in_dptr[i] <= 0) ? in_dptr[i] * alpha_dptr[c] : in_dptr[i];
  }
}

template<typename T>
__global__ void PReluDataBackward(const int64_t elem_cnt, const int64_t channel_num,
                                  const int64_t area, const T* in_dptr, const T* alpha_dptr,
                                  const T* out_dff_dptr, T* in_diff_dptr) {
  CUDA_1D_KERNEL_LOOP(i, elem_cnt) {
    int64_t c = (i / area) % channel_num;
    in_diff_dptr[i] = (in_dptr[i] <= 0) ? out_dff_dptr[i] * alpha_dptr[c] : out_dff_dptr[i];
  }
}

template<typename T>
__global__ void PReluAlphaBackward(const int64_t elem_cnt, const T* in_dptr, const T* out_diff_dptr,
                                   T* alpha_diff_buf_dptr) {
  CUDA_1D_KERNEL_LOOP(i, elem_cnt) {
    alpha_diff_buf_dptr[i] = (in_dptr[i] <= 0) ? out_diff_dptr[i] * in_dptr[i] : 0;
  }
}

}  // namespace

template<typename T>
struct PReluKernelUtil<DeviceType::kGPU, T> {
  static void Forward(const KernelCtx& ctx, const PReluOpConf& conf, const Blob* in_blob,
                      const Blob* alpha_blob, Blob* out_blob) {
    const int64_t elem_cnt = in_blob->shape().elem_cnt();
    if (conf.channel_shared()) {
      PReluForward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                     ctx.device_ctx->cuda_stream()>>>(
          elem_cnt, 1, 1, in_blob->dptr<T>(), alpha_blob->dptr<T>(), out_blob->mut_dptr<T>());
    } else {
      if (conf.data_format() == "channels_first") {
        const int64_t channel_num = in_blob->shape().At(1);
        const int64_t area = in_blob->shape().Count(2);
        PReluForward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                       ctx.device_ctx->cuda_stream()>>>(elem_cnt, channel_num, area,
                                                        in_blob->dptr<T>(), alpha_blob->dptr<T>(),
                                                        out_blob->mut_dptr<T>());
      } else if (conf.data_format() == "channels_last") {
        const int64_t channel_num = in_blob->shape().At(in_blob->shape().NumAxes() - 1);
        PReluForward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                       ctx.device_ctx->cuda_stream()>>>(elem_cnt, channel_num, 1,
                                                        in_blob->dptr<T>(), alpha_blob->dptr<T>(),
                                                        out_blob->mut_dptr<T>());
      } else {
        UNIMPLEMENTED();
      }
    }
  }

  static void Backward(const KernelCtx& ctx, const PReluOpConf& conf,
                       const PbRf<int32_t>& permutation, const Blob* in_blob,
                       const Blob* alpha_blob, const Blob* out_diff_blob, Blob* bw_buf_blob,
                       Blob* in_diff_blob, Blob* alpha_diff_blob) {
    const int64_t elem_cnt = out_diff_blob->shape().elem_cnt();
    // in_diff_blob acts as buffer here
    PReluAlphaBackward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                         ctx.device_ctx->cuda_stream()>>>(
        elem_cnt, in_blob->dptr<T>(), out_diff_blob->dptr<T>(), in_diff_blob->mut_dptr<T>());
    if (conf.channel_shared()) {
      KernelUtil<DeviceType::kGPU, T>::Sum(
          ctx.device_ctx, elem_cnt, in_diff_blob->dptr<T>(), alpha_diff_blob->mut_dptr<T>(),
          bw_buf_blob->mut_dptr<T>(), bw_buf_blob->ByteSizeOfDataContentField());
      PReluDataBackward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                          ctx.device_ctx->cuda_stream()>>>(
          elem_cnt, 1, 1, in_blob->dptr<T>(), alpha_blob->dptr<T>(), out_diff_blob->dptr<T>(),
          in_diff_blob->mut_dptr<T>());
    } else {
      KernelUtil<DeviceType::kGPU, T>::Transpose(
          ctx.device_ctx, in_diff_blob->shape().NumAxes(), in_diff_blob->shape(),
          bw_buf_blob->shape(), permutation, in_diff_blob->shape().elem_cnt(),
          in_diff_blob->dptr<T>(), bw_buf_blob->mut_dptr<T>());
      CHECK_EQ(elem_cnt, bw_buf_blob->shape().elem_cnt());
      if (conf.data_format() == "channels_first") {
        const int64_t channel_num = out_diff_blob->shape().At(1);
        CHECK_EQ(channel_num, bw_buf_blob->shape().At(0));
        KernelUtil<DeviceType::kGPU, T>::RowSum(
            ctx.device_ctx, channel_num, bw_buf_blob->shape().Count(1), bw_buf_blob->dptr<T>(),
            alpha_diff_blob->mut_dptr<T>(), in_diff_blob->mut_dptr<T>(),
            in_diff_blob->ByteSizeOfDataContentField());
        PReluDataBackward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                            ctx.device_ctx->cuda_stream()>>>(
            elem_cnt, channel_num, out_diff_blob->shape().Count(2), in_blob->dptr<T>(),
            alpha_blob->dptr<T>(), out_diff_blob->dptr<T>(), in_diff_blob->mut_dptr<T>());
      } else if (conf.data_format() == "channels_last") {
        const int64_t channel_num = out_diff_blob->shape().At(in_blob->shape().NumAxes() - 1);
        CHECK_EQ(channel_num, bw_buf_blob->shape().At(0));
        KernelUtil<DeviceType::kGPU, T>::RowSum(
            ctx.device_ctx, channel_num, bw_buf_blob->shape().Count(1), bw_buf_blob->dptr<T>(),
            alpha_diff_blob->mut_dptr<T>(), in_diff_blob->mut_dptr<T>(),
            in_diff_blob->ByteSizeOfDataContentField());
        PReluDataBackward<<<BlocksNum4ThreadsNum(elem_cnt), kCudaThreadsNumPerBlock, 0,
                            ctx.device_ctx->cuda_stream()>>>(
            elem_cnt, channel_num, 1, in_blob->dptr<T>(), alpha_blob->dptr<T>(),
            out_diff_blob->dptr<T>(), in_diff_blob->mut_dptr<T>());
      } else {
        UNIMPLEMENTED();
      }
    }
  }
};

#define INSTANTIATE_P_RELU_KERNEL_UTIL(type_cpp, type_proto) \
  template class PReluKernelUtil<DeviceType::kGPU, type_cpp>;
OF_PP_FOR_EACH_TUPLE(INSTANTIATE_P_RELU_KERNEL_UTIL, FLOATING_DATA_TYPE_SEQ);

}  // namespace oneflow