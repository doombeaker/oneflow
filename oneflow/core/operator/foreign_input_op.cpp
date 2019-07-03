#include "oneflow/core/operator/foreign_input_op.h"
#include "oneflow/core/job/sbp_signature_builder.h"

namespace oneflow {

namespace {

void CheckOpConf(const OperatorConf& op_conf) {
  CHECK(op_conf.ctrl_in_op_name().empty());
  if (op_conf.foreign_input_conf().blob_conf().has_dim0_inner_shape()) { TODO(); }
  if (op_conf.foreign_input_conf().blob_conf().has_dim1_valid_num()) { TODO(); }
  if (op_conf.foreign_input_conf().blob_conf().has_dim2_valid_num()) { TODO(); }
}

}  // namespace

void ForeignInputOp::InitFromOpConf() {
  CHECK(op_conf().has_foreign_input_conf());
  if (op_conf().foreign_input_conf().has_tick()) { EnrollOutputBn("tick", false); }
  EnrollOutputBn("out", false);
}

const PbMessage& ForeignInputOp::GetCustomizedConf() const {
  return op_conf().foreign_input_conf();
}

void ForeignInputOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                                    const ParallelContext* parallel_ctx) const {
  CHECK_EQ(parallel_ctx->parallel_num(), 1);
  CheckOpConf(op_conf());
  const auto& conf = op_conf().foreign_input_conf().blob_conf();
  BlobDesc* out_blob_desc = GetBlobDesc4BnInOp("out");
  out_blob_desc->mut_shape() = Shape(conf.shape());
  if (conf.has_data_type()) {
    out_blob_desc->set_data_type(conf.data_type());
  } else {
    out_blob_desc->set_data_type(GlobalJobDesc().DefaultDataType());
  }
  out_blob_desc->set_has_dim0_valid_num_field(conf.has_dim0_valid_num());
}

void ForeignInputOp::InferHasBatchDim(
    std::function<bool*(const std::string&)> HasBatchDim4BnInOp) const {
  *HasBatchDim4BnInOp("out") = op_conf().foreign_input_conf().blob_conf().has_batch_dim();
}

void ForeignInputOp::GetSbpSignatures(SbpSignatureList* sbp_sig_list) const {}

REGISTER_OP(OperatorConf::kForeignInputConf, ForeignInputOp);
REGISTER_OP_SAME_OUTPUT_BLOB_MEM_BLOCK_NUM(OperatorConf::kForeignInputConf, 1);

}  // namespace oneflow