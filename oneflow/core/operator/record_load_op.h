#ifndef ONEFLOW_CORE_OPERATOR_RECORD_LOAD_OP_H_
#define ONEFLOW_CORE_OPERATOR_RECORD_LOAD_OP_H_

#include "oneflow/core/operator/operator.h"
#include "oneflow/core/graph/logical_node.h"

namespace oneflow {

class RecordLoadOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(RecordLoadOp);
  RecordLoadOp() = default;
  ~RecordLoadOp() = default;

  void InitFromOpConf() override;
  const PbMessage& GetCustomizedConf() const override;
  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

  LogicalNode* NewProperLogicalNode() override { return new RecordLoadLogicalNode; }

 private:
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_RECORD_LOAD_OP_H_