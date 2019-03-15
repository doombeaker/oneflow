#include "oneflow/core/operator/relu_grad_op.h"

namespace oneflow {

void ReluGradOp::InitFromOpConf() {
  CHECK(op_conf().has_relu_grad_conf());
  EnrollInputBn("y");
  EnrollInputBn("dy");
  EnrollOutputBn("dx");
}

const PbMessage& ReluGradOp::GetCustomizedConf() const { return op_conf().relu_grad_conf(); }

void ReluGradOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                                const ParallelContext* parallel_ctx) const {
  *GetBlobDesc4BnInOp("dx") = *GetBlobDesc4BnInOp("y");
}

REGISTER_OP(OperatorConf::kReluGradConf, ReluGradOp);

}  // namespace oneflow