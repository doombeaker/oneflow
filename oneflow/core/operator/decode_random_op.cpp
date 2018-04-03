#include "oneflow/core/operator/decode_random_op.h"

namespace oneflow {

void DecodeRandomOp::InitFromOpConf() {
  CHECK(op_conf().has_decode_random_conf());
  EnrollOutputBn("out", false);
}

const PbMessage& DecodeRandomOp::GetCustomizedConf() const {
  return op_conf().decode_random_conf();
}

void DecodeRandomOp::VirtualGenKernelConf(
    std::function<const BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx, KernelConf* kernel_conf) const {
  kernel_conf->mutable_decode_random_conf()->set_random_seed(NewRandomSeed());
}

void DecodeRandomOp::InferBlobDescs(
    std::function<BlobDesc*(const std::string)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx) const {
  BlobDesc* out_blob_desc = GetBlobDesc4BnInOp("out");
  const DecodeRandomOpConf& conf = op_conf().decode_random_conf();
  std::vector<int64_t> dim_vec(1 + conf.shape().dim_size());
  dim_vec[0] = Global<JobDesc>::Get()->SinglePieceSize();
  FOR_RANGE(size_t, j, 1, dim_vec.size()) {
    dim_vec[j] = conf.shape().dim(j - 1);
  }
  out_blob_desc->mut_shape() = Shape(dim_vec);
  out_blob_desc->set_data_type(conf.data_type());
  out_blob_desc->set_has_data_id_field(Global<JobDesc>::Get()->SizeOfOneDataId()
                                       > 0);
  out_blob_desc->set_has_col_num_field(conf.max_sequence_size() > 1);
  out_blob_desc->set_max_col_num(conf.max_sequence_size());
}

REGISTER_OP(OperatorConf::kDecodeRandomConf, DecodeRandomOp);

}  // namespace oneflow