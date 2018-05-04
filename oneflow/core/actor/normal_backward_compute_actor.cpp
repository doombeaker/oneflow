#include "oneflow/core/actor/normal_backward_compute_actor.h"

namespace oneflow {

void NormalBackwardCompActor::VirtualCompActorInit(const TaskProto& task_proto) {
  b121_out_regst_desc_id_ = Name2SoleRegstDescId("boxing_out");
  if (b121_out_regst_desc_id_ == -1) { b121_out_regst_desc_id_ = Name2SoleRegstDescId("121_out"); }
  CHECK_NE(b121_out_regst_desc_id_, -1);
  model_regst_desc_id_ = Name2SoleRegstDescId("model");
  model_tmp_regst_desc_id_ = Name2SoleRegstDescId("model_tmp");
  model_tmp_regst_ = nullptr;
  staleness_ = -1;
  OF_SET_MSG_HANDLER(&NormalBackwardCompActor::HandlerNormal);
}

void NormalBackwardCompActor::ForEachCurCustomizedReadableRegst(
    std::function<void(const Regst*)> handler) {
  if (model_regst_desc_id_ != -1) {
    CHECK_EQ(model_regst_queue_.empty(), false);
    handler(model_regst_queue_.front());
  }
  if (model_tmp_regst_desc_id_ != -1) {
    CHECK(model_tmp_regst_);
    handler(model_tmp_regst_);
  }
}

void NormalBackwardCompActor::NormalProcessNaiveReadableRegstMsg(const std::deque<Regst*>& rq) {
  if (rq.size() == 1 && rq.front()->regst_desc_id() == b121_out_regst_desc_id_) {
    AsyncReturnModelRegstUntilModelVersionIdEqual(rq.front()->model_version_id());
  }
}

void NormalBackwardCompActor::NormalProcessCustomizedReadableRegstMsg(const ActorMsg& msg) {
  if (msg.regst()->regst_desc_id() == model_regst_desc_id_) {
    if (staleness_ == -1) { staleness_ = msg.regst()->regst_desc()->register_num() - 1; }
    model_regst_queue_.push(msg.regst());
  } else if (msg.regst()->regst_desc_id() == model_tmp_regst_desc_id_) {
    CHECK(model_tmp_regst_ == nullptr);
    model_tmp_regst_ = msg.regst();
  } else {
    UNIMPLEMENTED() << msg.regst()->regst_desc_id();
  }
}

void NormalBackwardCompActor::Act() {
  int64_t piece_id = GetNaiveCurReadable(b121_out_regst_desc_id_)->piece_id();
  AsyncLaunchKernel(GenDefaultKernelCtx(), [this](int64_t regst_desc_id) -> Regst* {
    if (regst_desc_id == model_regst_desc_id_) {
      return model_regst_queue_.front();
    } else if (regst_desc_id == model_tmp_regst_desc_id_) {
      return model_tmp_regst_;
    } else {
      return nullptr;
    }
  });
  AsyncSendRegstMsgToConsumer([&](Regst* regst) {
    regst->set_piece_id(piece_id);
    return true;
  });
  Regst* next_b121_out = GetNaiveNextReadable(b121_out_regst_desc_id_);
  if (next_b121_out == nullptr) {
    AsyncReturnModelRegstUntilLastPieceIdGreaterThan(piece_id);
  } else {
    AsyncReturnModelRegstUntilModelVersionIdEqual(next_b121_out->model_version_id());
  }
}

bool NormalBackwardCompActor::IsCustomizedReadReady() {
  if (model_regst_desc_id_ != -1) {
    if (model_regst_queue_.empty()) { return false; }
    int64_t expected_model_vid = GetNaiveCurReadable(b121_out_regst_desc_id_)->model_version_id();
    CHECK_EQ(expected_model_vid, model_regst_queue_.front()->model_version_id());
  }
  if (model_tmp_regst_desc_id_ != -1 && model_tmp_regst_ == nullptr) { return false; }
  return true;
}

void NormalBackwardCompActor::AsyncReturnAllCustomizedReadableRegst() {
  while (model_regst_queue_.empty() == false) {
    AsyncSendRegstMsgToProducer(model_regst_queue_.front());
    model_regst_queue_.pop();
  }
  if (model_tmp_regst_) {
    AsyncSendRegstMsgToProducer(model_tmp_regst_);
    model_tmp_regst_ = nullptr;
  }
}

void NormalBackwardCompActor::AsyncReturnModelRegstUntilModelVersionIdEqual(int64_t model_vid) {
  if (model_regst_desc_id_ == -1) { return; }
  while (!model_regst_queue_.empty()
         && model_regst_queue_.front()->model_version_id() < model_vid) {
    AsyncSendRegstMsgToProducer(model_regst_queue_.front());
    model_regst_queue_.pop();
  }
  if (!model_regst_queue_.empty()) {
    CHECK_EQ(model_regst_queue_.front()->model_version_id(), model_vid);
  }
}

void NormalBackwardCompActor::AsyncReturnModelRegstUntilLastPieceIdGreaterThan(int64_t piece_id) {
  if (model_regst_desc_id_ == -1) { return; }
  while (model_regst_queue_.empty() == false) {
    int64_t model_vid = model_regst_queue_.front()->model_version_id();
    int64_t last_piece_id = GetLastPieceIdForModelVersionId(staleness_, model_vid);
    if (last_piece_id > piece_id) { return; }
    AsyncSendRegstMsgToProducer(model_regst_queue_.front());
    model_regst_queue_.pop();
  }
}

REGISTER_ACTOR(TaskType::kNormalBackward, NormalBackwardCompActor);

}  // namespace oneflow