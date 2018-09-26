#ifndef ONEFLOW_CORE_DEVICE_NCCL_UTIL_H_
#define ONEFLOW_CORE_DEVICE_NCCL_UTIL_H_

#include "oneflow/core/register/blob.h"
#include "oneflow/core/common/data_type.pb.h"
#include "oneflow/core/common/util.h"

#ifdef WITH_NCCL
#include <nccl.h>
#endif  // WITH_NCCL

namespace oneflow {

#ifdef WITH_NCCL
inline ncclDataType_t GetNcclDataType(const DataType& dt) {
  switch (dt) {
#define NCCL_DATA_TYPE_CASE(dtype) \
  case DataType::k##dtype: return ncclDataType_t::nccl##dtype
    NCCL_DATA_TYPE_CASE(Char);
    NCCL_DATA_TYPE_CASE(Float);
    NCCL_DATA_TYPE_CASE(Double);
    NCCL_DATA_TYPE_CASE(Int8);
    NCCL_DATA_TYPE_CASE(Int32);
    default: UNIMPLEMENTED();
  }
}

void NcclCheck(ncclResult_t error);
#endif  // WITH_NCCL

class NcclUtil final {
 public:
  using NcclReduceMthd = void(DeviceCtx*, Blob*, Blob*);
  static void AllReduce(DeviceCtx* ctx, Blob* send_blob, Blob* recv_blob) {
#ifdef WITH_NCCL

    auto elem_cnt = (size_t)send_blob->shape().elem_cnt();
    NcclCheck(ncclAllReduce(send_blob->dptr(), recv_blob->mut_dptr(), elem_cnt,
                            GetNcclDataType(send_blob->data_type()), ncclSum, ctx->nccl_handle(),
                            ctx->cuda_stream()));
#else
    UNIMPLEMENTED();
#endif  // WITH_NCCL
  }

  static void ReduceScatter(DeviceCtx* ctx, Blob* send_blob, Blob* recv_blob) {
#ifdef WITH_NCCL

    auto elem_cnt = (size_t)recv_blob->shape().elem_cnt();
    NcclCheck(ncclReduceScatter(send_blob->dptr(), recv_blob->mut_dptr(), elem_cnt,
                                GetNcclDataType(send_blob->data_type()), ncclSum,
                                ctx->nccl_handle(), ctx->cuda_stream()));
#else
    UNIMPLEMENTED();
#endif  // WITH_NCCL
  }

  static void AllGather(DeviceCtx* ctx, Blob* send_blob, Blob* recv_blob) {
#ifdef WITH_NCCL

    auto elem_cnt = (size_t)send_blob->shape().elem_cnt();
    NcclCheck(ncclAllGather(send_blob->dptr(), recv_blob->mut_dptr(), elem_cnt,
                            GetNcclDataType(send_blob->data_type()), ctx->nccl_handle(),
                            ctx->cuda_stream()));
#else
    UNIMPLEMENTED();
#endif  // WITH_NCCL
  }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_DEVICE_NCCL_UTIL_H_