#include "oneflow/core/operator/shape_elem_cnt_op.h"

namespace oneflow {

namespace {

class ShapeElemCntOpSplitSbpSignature final : public ParallelSbpSignature {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ShapeElemCntOpSplitSbpSignature);
  ~ShapeElemCntOpSplitSbpSignature() override = default;

  explicit ShapeElemCntOpSplitSbpSignature(const Operator* op) : ParallelSbpSignature(op) {}

  const std::string Description() const override { return op().op_name() + ": (S,) -> (P,)"; }

  const SbpSigMatchResult GetMatchResult(
      const std::function<const SbpInferHint&(const std::string&)>& SbpInferHint4BnInOp,
      const ParallelDesc& parallel_desc) const override {
    const SbpInferHint& in_sbp_infer_hint = SbpInferHint4BnInOp("x");
    if (!in_sbp_infer_hint.sbp_parallel().has_split_parallel()) {
      return MakeSbpSigMatchSignatureMismatch();
    }
    const int32_t num_axes = SbpInferHint4BnInOp("x").num_axes();
    const int32_t split_axis = SbpInferHint4BnInOp("x").sbp_parallel().split_parallel().axis();
    const int32_t begin_axis = op().op_conf().shape_elem_cnt_conf().begin_axis();
    int32_t end_axis = op().op_conf().shape_elem_cnt_conf().end_axis();
    if (end_axis < 0) { end_axis += num_axes; }
    if (split_axis >= begin_axis && split_axis <= end_axis) {
      return MakeSbpSigMatchSuccess();
    } else {
      return MakeSbpSigMatchSignatureMismatch();
    }
  }

  void GenerateSignature(
      const std::function<const SbpInferHint&(const std::string&)>& SbpInferHint4BnInOp,
      HashMap<std::string, SbpParallel>* bn2sbp) const override {
    (*bn2sbp)["x"] = SbpInferHint4BnInOp("x").sbp_parallel();
    (*bn2sbp)["y"].mutable_partial_sum_parallel();
  }
};

class ShapeElemCntOpBroadcastSbpSignature final : public ParallelSbpSignature {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ShapeElemCntOpBroadcastSbpSignature);
  ~ShapeElemCntOpBroadcastSbpSignature() override = default;

  explicit ShapeElemCntOpBroadcastSbpSignature(const Operator* op) : ParallelSbpSignature(op) {}

  const std::string Description() const override { return op().op_name() + ": (S,) -> (B | P)"; }

  const SbpSigMatchResult GetMatchResult(
      const std::function<const SbpInferHint&(const std::string&)>& SbpInferHint4BnInOp,
      const ParallelDesc& parallel_desc) const override {
    const SbpInferHint& in_sbp_infer_hint = SbpInferHint4BnInOp("x");
    if (!in_sbp_infer_hint.sbp_parallel().has_split_parallel()) { return MakeSbpSigMatchSuccess(); }
    const int32_t num_axes = SbpInferHint4BnInOp("x").num_axes();
    const int32_t split_axis = SbpInferHint4BnInOp("x").sbp_parallel().split_parallel().axis();
    const int32_t begin_axis = op().op_conf().shape_elem_cnt_conf().begin_axis();
    int32_t end_axis = op().op_conf().shape_elem_cnt_conf().end_axis();
    if (end_axis < 0) { end_axis += num_axes; }
    if (split_axis >= begin_axis && split_axis <= end_axis) {
      return MakeSbpSigMatchSignatureMismatch();
    } else {
      return MakeSbpSigMatchSuccess();
    }
  }

  void GenerateSignature(
      const std::function<const SbpInferHint&(const std::string&)>& SbpInferHint4BnInOp,
      HashMap<std::string, SbpParallel>* bn2sbp) const override {
    (*bn2sbp)["x"].mutable_broadcast_parallel();
    (*bn2sbp)["y"].mutable_broadcast_parallel();
  }
};

}  // namespace

void ShapeElemCntOp::InitFromOpConf() {
  EnrollInputBn("x")->set_use_header_only(true);
  EnrollOutputBn("y");
}

const PbMessage& ShapeElemCntOp::GetCustomizedConf() const {
  return op_conf().shape_elem_cnt_conf();
}

void ShapeElemCntOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                                    const ParallelContext* parallel_ctx) const {
  GetBlobDesc4BnInOp("y")->set_data_type(DataType::kInt32);
  GetBlobDesc4BnInOp("y")->mut_shape() = Shape({1});
  const int32_t num_axes = GetBlobDesc4BnInOp("x")->shape().NumAxes();
  const int32_t begin_axis = op_conf().shape_elem_cnt_conf().begin_axis();
  CHECK_GE(begin_axis, 0);
  CHECK_LT(begin_axis, num_axes);
  int32_t end_axis = op_conf().shape_elem_cnt_conf().end_axis();
  if (end_axis < 0) { end_axis += num_axes; }
  CHECK_GE(end_axis, 0);
  CHECK_LT(end_axis, num_axes);
}

void ShapeElemCntOp::GetSbpSignatures(
    std::vector<std::unique_ptr<const SbpSignature>>* sbp_signature) const {
  sbp_signature->emplace_back(MakeBroadcastSbpSignature(this));
  sbp_signature->emplace_back(new ShapeElemCntOpSplitSbpSignature(this));
  sbp_signature->emplace_back(new ShapeElemCntOpBroadcastSbpSignature(this));
}

REGISTER_OP(OperatorConf::kShapeElemCntConf, ShapeElemCntOp);

}  // namespace oneflow