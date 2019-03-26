#include "oneflow/core/autograd/autograd.h"

namespace oneflow {

namespace {

void GenerateBackwardOpConf(
    const Operator& op, std::vector<OperatorConf>* op_confs,
    const std::function<LogicalBlobId*(const std::string&)>& DiffLbi4BnInOp) {
  CHECK(op.op_conf().has_fully_connected_conf());
  const auto& conf = op.op_conf().fully_connected_conf();
  if (DiffLbi4BnInOp("in") != nullptr) {
    OperatorConf matmul_in_op;
    matmul_in_op.set_name(op.op_name() + "_in_grad");
    MatmulOpConf* matmul_in_op_conf = matmul_in_op.mutable_matmul_conf();
    matmul_in_op_conf->set_out("out");
    matmul_in_op_conf->set_a(GenLogicalBlobName(*DiffLbi4BnInOp("out")));
    matmul_in_op_conf->set_b(GenLogicalBlobName(op.BnInOp2Lbi("weight")));
    matmul_in_op_conf->set_transpose_a(false);
    matmul_in_op_conf->set_transpose_b(false);
    op_confs->push_back(matmul_in_op);
    DiffLbi4BnInOp("in")->set_op_name(matmul_in_op.name());
    DiffLbi4BnInOp("in")->set_blob_name("out");
  }
  if (DiffLbi4BnInOp("weight") != nullptr) {
    OperatorConf matmul_weight_op;
    matmul_weight_op.set_name(op.op_name() + "_weight_grad");
    MatmulOpConf* matmul_weight_op_conf = matmul_weight_op.mutable_matmul_conf();
    matmul_weight_op_conf->set_out("out");
    matmul_weight_op_conf->set_a(GenLogicalBlobName(*DiffLbi4BnInOp("out")));
    matmul_weight_op_conf->set_b(GenLogicalBlobName(op.BnInOp2Lbi("in")));
    matmul_weight_op_conf->set_transpose_a(true);
    matmul_weight_op_conf->set_transpose_b(false);
    op_confs->push_back(matmul_weight_op);
    DiffLbi4BnInOp("weight")->set_op_name(matmul_weight_op.name());
    DiffLbi4BnInOp("weight")->set_blob_name("out");
  }
  if (conf.use_bias() && (DiffLbi4BnInOp("bias") != nullptr)) {
    OperatorConf reduce_sum_op;
    reduce_sum_op.set_name(op.op_name() + "_bias_grad");
    ReduceSumOpConf* reduce_sum_op_conf = reduce_sum_op.mutable_reduce_sum_conf();
    reduce_sum_op_conf->set_in(GenLogicalBlobName(*DiffLbi4BnInOp("out")));
    reduce_sum_op_conf->set_out("out");
    reduce_sum_op_conf->add_axis(0);
    reduce_sum_op_conf->set_keep_dims(false);
    op_confs->push_back(reduce_sum_op);
    DiffLbi4BnInOp("bias")->set_op_name(reduce_sum_op.name());
    DiffLbi4BnInOp("bias")->set_blob_name("out");
  }
}

}  // namespace

REGISTER_OP_GRAD(OperatorConf::kFullyConnectedConf, &GenerateBackwardOpConf);

}  // namespace oneflow