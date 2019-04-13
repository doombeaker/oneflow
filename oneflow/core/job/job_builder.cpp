#include "oneflow/core/job/job_builder.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/operator/operator.h"

namespace oneflow {

void SetBnValInOpTypeConf(PbMessage* pb_msg, const std::string& bn, const std::string& old_val,
                          const std::string& new_val) {
  const PbFd* fd = pb_msg->GetDescriptor()->FindFieldByName(bn);
  if (fd) {
    CHECK_EQ(GetValFromPbMessage<std::string>(*pb_msg, bn), old_val);
    SetValInPbMessage<std::string>(pb_msg, bn, new_val);
  } else {
    const std::pair<std::string, int32_t> prefix_idx = GenUnRepeatedBn(bn);
    CHECK_EQ(GetPbRpfFromPbMessage<std::string>(*pb_msg, prefix_idx.first).Get(prefix_idx.second),
             old_val);
    PbRpf<std::string>* rpf = MutPbRpfFromPbMessage<std::string>(pb_msg, prefix_idx.first);
    *rpf->Mutable(prefix_idx.second) = new_val;
  }
}

JobBuilder::JobBuilder(Job* job) : job_(job) {
  FOR_RANGE(int, i, 0, job->net().op_size()) {
    CHECK(op_name2op_conf_.emplace(job->net().op(i).name(), job->mutable_net()->mutable_op(i))
              .second);
  }
}

void JobBuilder::AddOps(const ParallelConf& parallel_conf,
                        const std::vector<OperatorConf>& op_confs) const {
  auto* placemnt_group = job_->mutable_placement()->add_placement_group();
  *placemnt_group->mutable_parallel_conf() = parallel_conf;
  for (const auto& op_conf : op_confs) {
    CHECK(op_name2op_conf_.find(op_conf.name()) == op_name2op_conf_.end());
    *job_->mutable_net()->add_op() = op_conf;
    placemnt_group->mutable_op_set()->add_op_name(op_conf.name());
  }
}

void JobBuilder::MutOps(const std::vector<OperatorConf>& op_confs) const {
  for (const auto& op_conf : op_confs) { op_name2op_conf_.at(op_conf.name())->CopyFrom(op_conf); }
}

void JobBuilder::AddOrMutOps(const ParallelConf& parallel_conf,
                             const std::vector<OperatorConf>& op_confs) const {
  std::vector<OperatorConf> add_ops;
  std::vector<OperatorConf> mut_ops;
  for (const auto& op_conf : op_confs) {
    if (op_name2op_conf_.find(op_conf.name()) == op_name2op_conf_.end()) {
      add_ops.push_back(op_conf);
    } else {
      mut_ops.push_back(op_conf);
    }
  }
  AddOps(parallel_conf, add_ops);
  MutOps(mut_ops);
}

}  // namespace oneflow