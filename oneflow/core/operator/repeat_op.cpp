#include "oneflow/core/graph/logical_node.h"
#include "oneflow/core/operator/repeat_op.h"

namespace oneflow {

void oneflow::RepeatOp::InitFromOpConf() {
  CHECK(op_conf().has_repeat_conf());
  CHECK_GE(op_conf().repeat_conf().repeat_num(), 1);
  EnrollInputBn("in");
  EnrollOutputBn("out");
}

const PbMessage& RepeatOp::GetCustomizedConf() const { return op_conf().repeat_conf(); }

void RepeatOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                              const ParallelContext* parallel_ctx) const {
  BlobDesc* in_blob_desc = GetBlobDesc4BnInOp("in");
  BlobDesc* out_blob_desc = GetBlobDesc4BnInOp("out");
  *out_blob_desc = *in_blob_desc;
}

void RepeatOp::InferDiffBlobDescsWithoutFwBlob(
    std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx) const {
  BlobDesc* in_diff_blob_desc = GetBlobDesc4BnInOp(GenDiffBn("in"));
  BlobDesc* out_diff_blob_desc = GetBlobDesc4BnInOp(GenDiffBn("out"));
  *in_diff_blob_desc = *out_diff_blob_desc;
}

LogicalNode* RepeatOp::NewProperLogicalNode() { return new RepeatForwardLogicalNode(); }

REGISTER_OP(OperatorConf::kRepeatConf, RepeatOp);

}  // namespace oneflow