#ifndef ONEFLOW_CORE_OPERATOR_KEEP_HEADER_ONLY_OP_H_
#define ONEFLOW_CORE_OPERATOR_KEEP_HEADER_ONLY_OP_H_

#include "oneflow/core/operator/identity_op.h"
#include "oneflow/core/graph/logical_node.h"

namespace oneflow {

class KeepHeaderOnlyOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(KeepHeaderOnlyOp);
  KeepHeaderOnlyOp() = default;
  ~KeepHeaderOnlyOp() override = default;

  void InitFromOpConf() override;
  bool NeedInBlobWhenBackward() const override { return false; }
  bool NeedOutBlobWhenBackward() const override { return false; }

  const PbMessage& GetCustomizedConf() const override { return op_conf().keep_header_only_conf(); }
  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

 private:
  bool IsInputBlobAllowedModelSplit(const std::string& ibn) const override { return true; }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_KEEP_HEADER_ONLY_OP_H_