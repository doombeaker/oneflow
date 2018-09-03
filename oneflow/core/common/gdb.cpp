#include "oneflow/core/register/blob.h"
#include "oneflow/core/kernel/kernel_util.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/common/protobuf.h"

namespace oneflow {

// used by gdb only
namespace gdb {

namespace {

static char* MallocThenCpyD2H(const char* gpu_src, size_t size) {
  char* cpu_dst = reinterpret_cast<char*>(malloc(size));
  cudaMemcpy(cpu_dst, gpu_src, size, cudaMemcpyDeviceToHost);
  return cpu_dst;
}

static void CpyH2DThenFree(char* gpu_dst, char* cpu_src, size_t size) {
  cudaMemcpy(gpu_dst, cpu_src, size, cudaMemcpyHostToDevice);
  free(cpu_src);
}

template<typename T>
void LoadFromStrFile(T* buf, const std::string& file_name) {
  std::ifstream file(file_name);
  CHECK(file.is_open());
  std::string line;
  for (int64_t i = 0; std::getline(file, line); ++i) { buf[i] = oneflow_cast<T>(line); }
  file.close();
}

}  // namespace

// used by passing std::string param
static std::string param0;

static void CudaMemCpyH2DThenFreeCpuPtr(uint64_t gpu_dst, uint64_t cpu_src, size_t size) {
  CpyH2DThenFree(reinterpret_cast<char*>(gpu_dst), reinterpret_cast<char*>(cpu_src), size);
}

static void* MallocCpuBufThenCudaMemCpyD2H(uint64_t gpu_src, size_t size) {
  return MallocThenCpyD2H(reinterpret_cast<char*>(gpu_src), size);
}

static void FloatBufLoadFromStrFile(uint64_t ptr, const char* file_name) {
  LoadFromStrFile(reinterpret_cast<float*>(ptr), std::string(file_name));
}

static void Int32BufLoadFromStrFile(uint64_t ptr, const char* file_name) {
  LoadFromStrFile(reinterpret_cast<int32_t*>(ptr), std::string(file_name));
}

static Blob* Blob4BnInOp(const std::function<Blob*(const std::string&)>* BnInOp2Blob,
                         const char* bn_in_op) {
  return (*BnInOp2Blob)(std::string(bn_in_op));
}

static HashMap<std::string, std::vector<std::string>> GetAllBlobNames(
    const OpAttribute& op_attribute) {
  std::list<std::string> attrs{
      "input_bns",         "input_diff_bns", "output_bns",   "output_diff_bns", "data_tmp_bns",
      "fw_buf_bns",        "bw_buf_bns",     "model_bns",    "model_diff_bns",  "const_model_bns",
      "forward_model_bns", "const_buf_bns",  "pb_input_bns", "pb_output_bns",
  };
  HashMap<std::string, std::vector<std::string>> ret;
  for (const auto& attr : attrs) {
    const auto& repeated_field = GetPbRpfFromPbMessage<std::string>(op_attribute, attr);
    if (repeated_field.empty() == false) { ret.insert({attr, PbRpf2StdVec(repeated_field)}); }
  }
  return ret;
}

void ForwardEnterBreakPoint(const OpAttribute& op_attribute,
                            const std::function<Blob*(const std::string&)>& BnInOp2Blob) {
  // do nothing
}

void ForwardLeaveBreakPoint(const OpAttribute& op_attribute,
                            const std::function<Blob*(const std::string&)>& BnInOp2Blob) {
  // do nothing
}

void BackwardEnterBreakPoint(const OpAttribute& op_attribute,
                             const std::function<Blob*(const std::string&)>& BnInOp2Blob) {
  // do nothing
}

void BackwardLeaveBreakPoint(const OpAttribute& op_attribute,
                             const std::function<Blob*(const std::string&)>& BnInOp2Blob) {
  // do nothing
}

}  // namespace gdb

}  // namespace oneflow