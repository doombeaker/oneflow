#include "oneflow/core/autograd/autograd.h"

namespace oneflow {

namespace {

void GenerateBackwardOpConf(
    const Operator& op, std::vector<OperatorConf>* op_confs,
    const std::function<LogicalBlobId*(const std::string&)>& DiffLbi4BnInOp) {
  CHECK(op.op_conf().has_layer_norm_conf());
  const LayerNormOpConf& conf = op.op_conf().layer_norm_conf();
  if (conf.center()) { CHECK(conf.has_beta()); }
  if (conf.scale()) { CHECK(conf.has_gamma()); }
  LogicalBlobId grad_dy_lbi = *DiffLbi4BnInOp("out");
  const bool has_beta_diff = conf.has_beta() && (DiffLbi4BnInOp("beta") != nullptr);
  const bool has_gamma_diff = conf.has_gamma() && (DiffLbi4BnInOp("gamma") != nullptr);
  const bool need_scale_out_diff = conf.has_gamma() && (DiffLbi4BnInOp("in") != nullptr);
  if (has_beta_diff || has_gamma_diff || need_scale_out_diff) {
    OperatorConf param_grad_op;
    param_grad_op.set_name(op.op_name() + "_param_grad");
    LayerNormParamGradOpConf* param_grad_conf = param_grad_op.mutable_layer_norm_param_grad_conf();
    param_grad_conf->set_begin_params_axis(conf.begin_params_axis());
    param_grad_conf->set_dy(GenLogicalBlobName(*DiffLbi4BnInOp("out")));
    if (has_beta_diff) {
      param_grad_conf->set_beta_diff("beta_diff");
      DiffLbi4BnInOp("beta")->set_op_name(param_grad_op.name());
      DiffLbi4BnInOp("beta")->set_blob_name("beta_diff");
    }
    if (has_gamma_diff || need_scale_out_diff) {
      param_grad_conf->set_gamma(GenLogicalBlobName(op.BnInOp2Lbi("gamma")));
    }
    if (has_gamma_diff) {
      param_grad_conf->set_normalized(GenLogicalBlobName(op.BnInOp2Lbi("normalized")));
      param_grad_conf->set_gamma_diff("gamma_diff");
      DiffLbi4BnInOp("gamma")->set_op_name(param_grad_op.name());
      DiffLbi4BnInOp("gamma")->set_blob_name("gamma_diff");
    }
    if (need_scale_out_diff) {
      param_grad_conf->set_normalized_diff("normalized_diff");
      grad_dy_lbi.set_op_name(param_grad_op.name());
      grad_dy_lbi.set_blob_name("normalized_diff");
    }
    op_confs->push_back(param_grad_op);
  }
  if (DiffLbi4BnInOp("in") != nullptr) {
    OperatorConf grad_op;
    grad_op.set_name(op.op_name() + "_grad");
    LayerNormGradOpConf* grad_conf = grad_op.mutable_layer_norm_grad_conf();
    grad_conf->set_begin_norm_axis(conf.begin_norm_axis());
    grad_conf->set_epsilon(conf.epsilon());
    grad_conf->set_dy(GenLogicalBlobName(grad_dy_lbi));
    grad_conf->set_x(GenLogicalBlobName(op.BnInOp2Lbi("in")));
    grad_conf->set_mean(GenLogicalBlobName(op.BnInOp2Lbi("mean")));
    grad_conf->set_inv_variance(GenLogicalBlobName(op.BnInOp2Lbi("inv_variance")));
    grad_conf->set_dx("dx");
    DiffLbi4BnInOp("in")->set_op_name(grad_op.name());
    DiffLbi4BnInOp("in")->set_blob_name("dx");
    op_confs->push_back(grad_op);
  }
}

}  // namespace

REGISTER_OP_GRAD(OperatorConf::kLayerNormConf, &GenerateBackwardOpConf);

}  // namespace oneflow