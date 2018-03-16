#include "oneflow/core/actor/record_load_actor.h"
#include "oneflow/core/persistence/cyclic_persistent_in_stream.h"
#include "oneflow/core/persistence/normal_persistent_in_stream.h"
#include "oneflow/core/job/runtime_context.h"
#include "oneflow/core/record/record.pb.h"

namespace oneflow {

void RecordLoadActor::VirtualCompActorInit(const TaskProto& task_proto) {
  piece_id_ = 0;
  is_eof_ = false;
  OF_SET_MSG_HANDLER(&RecordLoadActor::HandlerWaitToStart);
  if (JobDesc::Singleton()->IsTrain()) {
    in_stream_.reset(
        new CyclicPersistentInStream(GlobalFS(), task_proto.data_path()));
  } else {
    in_stream_.reset(
        new NormalPersistentInStream(GlobalFS(), task_proto.data_path()));
  }
}

int RecordLoadActor::HandlerWaitToStart(const ActorMsg& msg) {
  CHECK_EQ(msg.actor_cmd(), ActorCmd::kStart);
  ActUntilFail();
  OF_SET_MSG_HANDLER(&RecordLoadActor::HandlerNormal);
  return 0;
}

int RecordLoadActor::HandlerNormal(const ActorMsg& msg) {
  CHECK_EQ(msg.msg_type(), ActorMsgType::kRegstMsg);
  CHECK_EQ(TryUpdtStateAsProducedRegst(msg.regst()), 0);
  ActUntilFail();
  return TrySwitchToZombieOrFinish();
}

void RecordLoadActor::Act() {
  Regst* regst = GetCurSoleWriteableRegst();
  regst->set_piece_id(piece_id_++);
  RecordBlobIf* blob = regst->GetRecordBlobIf();
  blob->ReadFrom(in_stream_.get());
  if (blob->record_num() < JobDesc::Singleton()->SinglePieceSize()) {
    is_eof_ = true;
  }
  if (blob->record_num() > 0) { AsyncSendRegstMsgToConsumer(); }
}

bool RecordLoadActor::IsReadReady() {
  return !is_eof_ && piece_id_ < RuntimeCtx::Singleton()->total_piece_num();
}

REGISTER_ACTOR(kRecordLoad, RecordLoadActor);

}  // namespace oneflow