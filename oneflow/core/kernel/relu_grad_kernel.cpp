#include "oneflow/core/kernel/relu_grad_kernel.h"

namespace oneflow {

template<DeviceType device_type, typename T>
void ReluGradKernel<device_type, T>::ForwardDataContent(
    const KernelCtx& ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  Blob* y_blob = BnInOp2Blob("y");
  KernelUtil<device_type, T>::ReluBackward(
      ctx.device_ctx, y_blob->shape().elem_cnt(), y_blob->dptr<T>(), y_blob->dptr<T>(),
      BnInOp2Blob("dy")->dptr<T>(), BnInOp2Blob("dx")->mut_dptr<T>());
}

ADD_DEFAULT_KERNEL_CREATOR(OperatorConf::kReluGradConf, ReluGradKernel, FLOATING_DATA_TYPE_SEQ);

}  // namespace oneflow