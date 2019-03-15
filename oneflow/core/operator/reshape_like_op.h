#ifndef ONEFLOW_CORE_OPERATOR_RESHAPE_LIKE_OP_H_
#define ONEFLOW_CORE_OPERATOR_RESHAPE_LIKE_OP_H_

#include "oneflow/core/operator/operator.h"

namespace oneflow {

class ReshapeLikeOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ReshapeLikeOp);
  ReshapeLikeOp() = default;
  ~ReshapeLikeOp() = default;

  void InitFromOpConf() override;
  const PbMessage& GetCustomizedConf() const override;
  // It could be inplace but won't pass the check for sole input
  bool IsForwardInplace() const override { return false; }

  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

 private:
  bool IsInputBlobAllowedModelSplit(const std::string& ibn) const override { return false; }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_RESHAPE_LIKE_OP_H_