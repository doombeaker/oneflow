#include "oneflow/core/operator/callback_notify_op.h"
#include "oneflow/core/graph/logical_node.h"
#include "oneflow/core/job/sbp_signature_builder.h"

namespace oneflow {

void CallbackNotifyOp::InitFromOpConf() {
  CHECK(op_conf().has_callback_notify_conf());
  EnrollInputBn("in", false);
}

LogicalNode* CallbackNotifyOp::NewProperLogicalNode() const {
  return new CallbackNotifyLogicalNode();
}

const PbMessage& CallbackNotifyOp::GetCustomizedConf() const {
  return op_conf().callback_notify_conf();
}

void CallbackNotifyOp::InferBlobDescs(
    std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx) const {
  CHECK_EQ(parallel_ctx->parallel_num(), 1);
  CHECK(GetBlobDesc4BnInOp("in")->shape() == Shape({1}));
  CHECK(IsIntegralDataType(GetBlobDesc4BnInOp("in")->data_type()));
}

void CallbackNotifyOp::InferHasBatchDim(
    std::function<bool*(const std::string&)> HasBatchDim4BnInOp) const {}

void CallbackNotifyOp::GetSbpSignatures(SbpSignatureList* sbp_sig_list) const {
  SbpSignatureBuilder().Split(input_bns(), 0).Build(sbp_sig_list->mutable_sbp_signature()->Add());
}

REGISTER_CPU_OP(OperatorConf::kCallbackNotifyConf, CallbackNotifyOp);

}  // namespace oneflow