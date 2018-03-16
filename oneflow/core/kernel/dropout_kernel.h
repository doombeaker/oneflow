#ifndef ONEFLOW_CORE_KERNEL_DROPOUT_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_DROPOUT_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
#include "oneflow/core/kernel/kernel_context.h"
#include "oneflow/core/kernel/random_generator.h"

namespace oneflow {

template<DeviceType device_type, typename T>
class DropoutKernel final : public KernelIf<device_type> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(DropoutKernel);
  DropoutKernel() = default;
  ~DropoutKernel() = default;

 private:
  void VirtualKernelInit(const ParallelContext*) override;
  void ForwardDataContent(
      const KernelCtx&,
      std::function<Blob*(const std::string&)>) const override;
  void BackwardDataContent(
      const KernelCtx&,
      std::function<Blob*(const std::string&)>) const override;

  // random_mask = random_uniform(0, 1)
  // y = dropout(x, random_mask, dropout_rate)
  void Dropout(DeviceCtx* ctx, const int64_t n, double dropout_rate, const T* x,
               float* random_mask, T* y) const;
  // y = dropout(x, random_mask)
  void DropoutBackward(DeviceCtx* ctx, const int64_t n, double dropout_rate,
                       const T* dy, const float* random_mask, T* dx) const;
  std::unique_ptr<RandomGenerator> random_generator_;
};

template<DeviceType device_type, typename T>
struct DropoutKernelUtil final {
  static void MaskAndScale(DeviceCtx* ctx, const int64_t n, double threshold,
                           double scale, const T* x, const float* random_mask,
                           T* y);
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_DROPOUT_KERNEL_H_