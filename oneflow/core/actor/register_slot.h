#ifndef ONEFLOW_CORE_ACTOR_REGISTER_SLOT_H_
#define ONEFLOW_CORE_ACTOR_REGISTER_SLOT_H_

#include "oneflow/core/register/register_manager.h"

namespace oneflow {

class RegstSlot final {
 public:
  OF_DISALLOW_COPY_AND_MOVE(RegstSlot);
  RegstSlot() : regst_desc_id2regsts_(), available_regst_desc_cnt_(0), is_inited_(false) {}
  ~RegstSlot() = default;

  bool is_inited() const { return is_inited_; }
  size_t total_regst_desc_cnt() const { return regst_desc_id2regsts_.size(); }
  size_t available_regst_desc_cnt() const { return available_regst_desc_cnt_; }

  bool IsCurSlotReady() const { return available_regst_desc_cnt() == total_regst_desc_cnt(); }
  bool HasRegstDescId(int64_t regst_desc_id) const;
  const std::deque<Regst*>& RegstDeq4RegstDescId(int64_t regst_desc_id) const;
  void ForEachFrontRegst(std::function<void(const Regst*)>) const;
  void ForEachRegstDeq(std::function<void(const std::deque<Regst*>&)>) const;
  void ForChosenFrontRegst(std::function<bool(int64_t)>, std::function<void(const Regst*)>) const;
  void ForChosenRegstDeq(std::function<bool(int64_t)>,
                         std::function<void(const std::deque<Regst*>&)>) const;

  Regst* Front(int64_t regst_desc_id) const;
  Regst* SoleFront() const;
  Regst* FirstFront() const;

  // 0: success, -1: cannot find regst_desc_id
  int TryPushBackRegst(Regst* regst);
  int TryPopFrontRegst(int64_t regst_desc_id);

  void InitedDone();
  void InsertRegstDescId(int64_t regst_desc_id);

 private:
  HashMap<int64_t, std::deque<Regst*>> regst_desc_id2regsts_;
  size_t available_regst_desc_cnt_;
  bool is_inited_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_ACTOR_REGISTER_SLOT_H_