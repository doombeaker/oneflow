#ifndef ONEFLOW_CORE_ACTOR_REENTRANT_LOCK_COMPUTE_ACTOR_H_
#define ONEFLOW_CORE_ACTOR_REENTRANT_LOCK_COMPUTE_ACTOR_H_

#include "oneflow/core/actor/compute_actor.h"
#include "oneflow/core/kernel/reentrant_lock_kernel.h"

namespace oneflow {

class ReentrantLockCompActor final : public CompActor {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ReentrantLockCompActor);
  ReentrantLockCompActor() = default;
  ~ReentrantLockCompActor() override = default;

 protected:
  void VirtualCompActorInit(const TaskProto&) override;

 private:
  void Act() override;
  void NormalProcessCustomizedReadableRegstMsg(const ActorMsg&) override;
  void ForEachCurCustomizedReadableRegst(std::function<void(const Regst*)>) const override;
  bool IsCustomizedReadReady() override;
  void NormalProcessCustomizedEordMsg(const ActorMsg&) override {}
  bool IsCustomizedReadAlwaysUnReadyFromNow() override;
  void AsyncReturnAllCustomizedReadableRegst() override;
  std::pair<RegstNameType, HashSet<std::string>> GetNaiveOrCustomizedConsumedRegstDescName()
      override {
    return std::make_pair(RegstNameType::kNaive, HashSet<std::string>{});
  }
  void VirtualAsyncSendNaiveProducedRegstMsgToConsumer() override;
  void AsyncSendCustomizedConsumedRegstMsgToProducer() override;
  int64_t GetCurProcessedRegstDescId();

  const std::string& Ibn4RegstDescId(int64_t id) const { return regst_desc_id2ibn_.at(id); }

  RegstSlot consumed_rs_;
  int64_t cur_processed_regst_desc_id_;
  HashMap<int64_t, std::string> regst_desc_id2ibn_;
  ReentrantLockStatus reentrant_lock_status_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_ACTOR_REENTRANT_LOCK_COMPUTE_ACTOR_H_