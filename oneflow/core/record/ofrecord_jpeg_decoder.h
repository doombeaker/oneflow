#ifndef ONEFLOW_CORE_RECORD_OFRECORD_JPEG_DECODER_H_
#define ONEFLOW_CORE_RECORD_OFRECORD_JPEG_DECODER_H_

#include "oneflow/core/record/ofrecord_decoder.h"

namespace oneflow {

template<typename T>
class OFRecordDecoderImpl<EncodeCase::kJpeg, T> final
    : public OFRecordDecoder<EncodeCase::kJpeg, T> {
 public:
 private:
  int32_t GetColNumOfFeature(const Feature&,
                             int64_t one_col_elem_num) const override;
  void ReadOneCol(DeviceCtx*, const Feature&, const BlobConf&, int32_t col_id,
                  T* out_dptr, int64_t one_col_elem_num) const override;
  void DecodeImage(const std::string& src_data,
                   std::vector<uint8_t>* image_data, Shape* image_shape) const;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_RECORD_OFRECORD_JPEG_DECODER_H_