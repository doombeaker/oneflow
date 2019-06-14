#ifndef ONEFLOW_CORE_OPERATOR_CASE_OP_H_
#define ONEFLOW_CORE_OPERATOR_CASE_OP_H_

#include "oneflow/core/operator/operator.h"

namespace oneflow {

class CaseOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CaseOp);
  CaseOp() = default;
  ~CaseOp() override = default;

  void InitFromOpConf() override;
  const PbMessage& GetCustomizedConf() const override;
  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

 private:
  void InferHasBatchDim(std::function<bool*(const std::string&)> HasBatchDim4BnInOp) const override;
  void GetSbpSignatures(
      const std::function<const BlobDesc&(const std::string&)>& LogicalBlobDesc4Ibn,
      SbpSignatureList* sbp_sig_list) const override;
  LogicalNode* NewProperLogicalNode() const override;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_CASE_OP_H_