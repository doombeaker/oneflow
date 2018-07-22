#include "oneflow/core/operator/record_load_op.h"

namespace oneflow {

void RecordLoadOp::InitFromOpConf() {
  CHECK(op_conf().has_record_load_conf());
  EnrollOutputBn("out", false);
}

const PbMessage& RecordLoadOp::GetCustomizedConf() const { return op_conf().record_load_conf(); }

void RecordLoadOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                                  const ParallelContext* parallel_ctx) const {
  BlobDesc* out_blob_desc = GetBlobDesc4BnInOp("out");
  int64_t global_piece_size = Global<JobDesc>::Get()->PieceSize();
  CHECK_EQ(global_piece_size % parallel_ctx->parallel_num(), 0);
  out_blob_desc->mut_shape() = Shape({global_piece_size / parallel_ctx->parallel_num()});
  out_blob_desc->set_data_type(kOFRecord);
}

REGISTER_OP(OperatorConf::kRecordLoadConf, RecordLoadOp);

}  // namespace oneflow