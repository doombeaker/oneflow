#ifndef ONEFLOW_CORE_KERNEL_POOLING_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_POOLING_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
#include "oneflow/core/device/cudnn_util.h"
#include "oneflow/core/common/eigen_util.h"

namespace oneflow {

#ifdef WITH_CUDA
class CudnnPoolingDesc final {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CudnnPoolingDesc);
  CudnnPoolingDesc() = delete;
  ~CudnnPoolingDesc();

  CudnnPoolingDesc(cudnnPoolingMode_t pooling_mode, int dims, const int* window,
                   const int* padding, const int* stride);

  const cudnnPoolingDescriptor_t& Get() const { return val_; }

 private:
  cudnnPoolingDescriptor_t val_;
};
#endif  // WITH_CUDA

class PoolingCtx final {
 public:
  PoolingCtx(const PoolingKernelConf&
#ifdef WITH_CUDA
             ,
             cudnnPoolingMode_t, DataType
#endif  // WITH_CUDA
  );
  ~PoolingCtx() = default;

  const PoolingKernelConf& kernel_conf() const { return kernel_conf_; }

#ifdef WITH_CUDA
  const cudnnTensorDescriptor_t& cudnn_in_tensor_desc() const;
  const cudnnTensorDescriptor_t& cudnn_out_tensor_desc() const;
  const cudnnPoolingDescriptor_t& cudnn_pooling_desc() const;
#endif  // WITH_CUDA

 private:
  std::vector<int> GetStdVecFromShapeInKernelConf(
      const std::string& field_name) const;
  PoolingKernelConf kernel_conf_;

#ifdef WITH_CUDA
  cudnnPoolingMode_t pooling_mode_;
  std::unique_ptr<CudnnTensorDesc> in_desc_;
  std::unique_ptr<CudnnTensorDesc> out_desc_;
  std::unique_ptr<CudnnPoolingDesc> pooling_desc_;
#endif  // WITH_CUDA
};

template<DeviceType device_type, typename T>
class PoolingKernelIf : public KernelIf<device_type> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(PoolingKernelIf);
  PoolingKernelIf() = default;
  virtual ~PoolingKernelIf() = default;

 protected:
#ifdef WITH_CUDA
  virtual cudnnPoolingMode_t GetCudnnPoolingMode() = 0;
#endif  // WITH_CUDA
  const PoolingCtx& pooling_ctx() const { return *pooling_ctx_; }
  void VirtualKernelInit(const ParallelContext*) override {
    pooling_ctx_.reset(new PoolingCtx(GetPoolingKernelConf()
#ifdef WITH_CUDA
                                          ,
                                      this->GetCudnnPoolingMode(),
                                      GetDataType<T>::val
#endif  // WITH_CUDA
                                      ));
  }
  virtual const PoolingKernelConf& GetPoolingKernelConf() const = 0;
  void ForwardDataContent(
      const KernelCtx& kernel_ctx,
      std::function<Blob*(const std::string&)> BnInOp2Blob) const override {
    const Blob* in_blob = BnInOp2Blob("in");
    Blob* out_blob = BnInOp2Blob("out");
    PoolingForward(kernel_ctx, this->pooling_ctx(), in_blob, out_blob);
  }
  void BackwardDataContent(
      const KernelCtx& kernel_ctx,
      std::function<Blob*(const std::string&)> BnInOp2Blob) const override {
    Blob* in_diff_blob = BnInOp2Blob("in_diff");
    if (in_diff_blob == nullptr) { return; }
    Memset<device_type>(kernel_ctx.device_ctx, in_diff_blob->mut_dptr(), 0,
                        in_diff_blob->ByteSizeOfDataContentField());
    const Blob* out_diff_blob = BnInOp2Blob("out_diff");
    const Blob* in_blob = BnInOp2Blob("in");
    const Blob* out_blob = BnInOp2Blob("out");
    PoolingBackward(kernel_ctx, this->pooling_ctx(), out_diff_blob, out_blob,
                    in_blob, in_diff_blob);
  }
  virtual void PoolingForward(const KernelCtx& kernel_ctx,
                              const PoolingCtx& pooling_ctx,
                              const Blob* in_blob, Blob* out_blob) const = 0;
  virtual void PoolingBackward(const KernelCtx& kernel_ctx,
                               const PoolingCtx& pooling_ctx,
                               const Blob* out_diff_blob, const Blob* out_blob,
                               const Blob* in_blob,
                               Blob* in_diff_blob) const = 0;

  std::unique_ptr<PoolingCtx> pooling_ctx_;
};

template<DeviceType device_type, typename T>
class PoolingKernel;

template<typename T>
class PoolingKernel<DeviceType::kCPU, T>
    : public PoolingKernelIf<DeviceType::kCPU, T> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(PoolingKernel);
  PoolingKernel() = default;
  virtual ~PoolingKernel() = default;

 protected:
  void PoolingForward(const KernelCtx& kernel_ctx,
                      const PoolingCtx& pooling_ctx, const Blob* in_blob,
                      Blob* out_blob) const override;
  void PoolingBackward(const KernelCtx& kernel_ctx,
                       const PoolingCtx& pooling_ctx, const Blob* out_diff_blob,
                       const Blob* out_blob, const Blob* in_blob,
                       Blob* in_diff_blob) const override;
  virtual T ForwardInitialize() const = 0;
  virtual void NCDHWProcess(const T& lhs, T& rhs) const = 0;
  virtual void NDHWCProcess(const int64_t in_col, const int64_t out_col,
                            ConstEigenMatrixMap<T>& in_mat,
                            EigenMatrixMap<T>& out_mat) const = 0;
  virtual void NCDHWFinalize(const int64_t size, T& out) const = 0;
  virtual void NDHWCFinalize(const int64_t size, const int64_t col,
                             EigenMatrixMap<T>& out_mat) const = 0;
  virtual void NCDHWProcessGrad(const T& in, const T& out, const T& out_diff,
                                const int64_t size, T& in_diff) const = 0;
  virtual void NDHWCProcessGrad(const int64_t out_col, const int64_t in_col,
                                const int64_t size,
                                ConstEigenArrayMap<T>& out_arr,
                                ConstEigenArrayMap<T>& in_arr,
                                ConstEigenArrayMap<T>& out_diff_arr,
                                EigenArrayMap<T>& in_diff_arr) const = 0;
  void ForwardNCDHW(const PoolingCtx& pooling_ctx, const Blob* in_blob,
                    Blob* out_blob) const;
  void BackwardNCDHW(const PoolingCtx& pooling_ctx, const Blob* out_diff_blob,
                     const Blob* out_blob, const Blob* in_blob,
                     Blob* in_diff_blob) const;
  void ForwardNDHWC(const PoolingCtx& pooling_ctx, const Blob* in_blob,
                    Blob* out_blob) const;
  void BackwardNDHWC(const PoolingCtx& pooling_ctx, const Blob* out_diff_blob,
                     const Blob* out_blob, const Blob* in_blob,
                     Blob* in_diff_blob) const;
};

template<typename T>
class PoolingKernel<DeviceType::kGPU, T>
    : public PoolingKernelIf<DeviceType::kGPU, T> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(PoolingKernel);
  PoolingKernel() = default;
  virtual ~PoolingKernel() = default;

 protected:
  void PoolingForward(const KernelCtx& kernel_ctx,
                      const PoolingCtx& pooling_ctx, const Blob* in_blob,
                      Blob* out_blob) const override;
  void PoolingBackward(const KernelCtx& kernel_ctx,
                       const PoolingCtx& pooling_ctx, const Blob* out_diff_blob,
                       const Blob* out_blob, const Blob* in_blob,
                       Blob* in_diff_blob) const override;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_POOLING_KERNEL_H_