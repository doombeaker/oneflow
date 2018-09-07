#ifndef ONEFLOW_CORE_KERNEL_MATMUL_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_MATMUL_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
namespace oneflow {

template<DeviceType device_type, typename T>
class MatmulKernel final : public KernelIfWithModel<device_type, T> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(MatmulKernel);
  MatmulKernel() = default;
  ~MatmulKernel() = default;

 private:
  void ForwardDataContent(const KernelCtx&,
                          std::function<Blob*(const std::string&)>) const override;
  void BackwardDataContent(const KernelCtx&,
                           std::function<Blob*(const std::string&)>) const override;
  void InitConstBufBlobs(DeviceCtx*,
                         std::function<Blob*(const std::string&)> BnInOp2Blob) const override;
  const PbMessage& GetCustomizedOpConf() const override;
};

}  // namespace oneflow

#endif  // ONEFLOE_CORE_KERNEL_MATMUL_KERNEL_H_