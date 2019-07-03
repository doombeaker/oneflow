#include "oneflow/core/operator/foreign_output_op.h"
#include "oneflow/core/job/sbp_signature_builder.h"

namespace oneflow {

void ForeignOutputOp::InitFromOpConf() {
  CHECK(op_conf().has_foreign_output_conf());
  EnrollInputBn("in");
}

void ForeignOutputOp::InferBlobDescs(
    std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx) const {
  CHECK_EQ(parallel_ctx->parallel_num(), 1);
}

const PbMessage& ForeignOutputOp::GetCustomizedConf() const {
  return op_conf().foreign_output_conf();
}

void ForeignOutputOp::InferHasBatchDim(
    std::function<bool*(const std::string&)> HasBatchDim4BnInOp) const {}

void ForeignOutputOp::GetSbpSignatures(
    const std::function<const BlobDesc&(const std::string&)>& LogicalBlobDesc4Ibn,
    SbpSignatureList* sbp_sig_list) const {}

REGISTER_OP(OperatorConf::kForeignOutputConf, ForeignOutputOp);

}  // namespace oneflow