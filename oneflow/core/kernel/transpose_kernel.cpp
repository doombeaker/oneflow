#include "oneflow/core/kernel/transpose_kernel.h"

namespace oneflow {

template<DeviceType device_type, typename T>
void TransposeKernel<device_type, T>::ForwardDataContent(
    const KernelCtx& ctx,
    std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  BnInOp2Blob("in")->Transpose(ctx.device_ctx, BnInOp2Blob("out"),
                               this->kernel_conf().transpose_conf().perm());
}

template<DeviceType device_type, typename T>
void TransposeKernel<device_type, T>::BackwardDataContent(
    const KernelCtx& ctx,
    std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  BnInOp2Blob("out_diff")
      ->Transpose(ctx.device_ctx, BnInOp2Blob("in_diff"),
                  this->kernel_conf().transpose_conf().invert_perm());
}

ADD_DEFAULT_KERNEL_CREATOR(OperatorConf::kTransposeConf, TransposeKernel,
                           ARITHMETIC_DATA_TYPE_SEQ);

}  // namespace oneflow