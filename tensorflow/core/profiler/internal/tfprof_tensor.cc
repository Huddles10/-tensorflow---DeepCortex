/* Copyright 2016 The TensorFlow Authors All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/profiler/internal/tfprof_tensor.h"

#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

namespace tensorflow {
namespace tfprof {
void TFProfTensor::Display(string* formatted_str,
                           TFProfTensorProto* tfprof_tensor_pb) {
  if (formatted_str) {
    if (formatted_str_.length() >= kTFProfTenosrMaxDisplayLen) {
      *formatted_str =
          absl::StrCat(formatted_str_, "...omitted from display\n\n");
    } else {
      *formatted_str = formatted_str_;
    }
  }
  if (tfprof_tensor_pb) {
    tfprof_tensor_pb->MergeFrom(tfprof_tensor_pb_);
  }
}

void TFProfTensor::Build() {
  tfprof_tensor_pb_.set_dtype(tensor_->dtype());

  switch (tensor_->dtype()) {
    // Double for all floats.
    case DataType::DT_FLOAT:
    case DataType::DT_DOUBLE: {
      std::vector<double> values_vec;
      if (tensor_->dtype() == DataType::DT_FLOAT) {
        GetValueVec<float, double>(&values_vec);
      } else if (tensor_->dtype() == DataType::DT_DOUBLE) {
        GetValueVec<double, double>(&values_vec);
      }
      BuildOutput<double>(0, 0, values_vec, &tfprof_tensor_pb_);
      break;
    }
    // Int64 for all integers.
    case DataType::DT_INT32:
    case DataType::DT_INT64: {
      std::vector<int64_t> values_vec;
      if (tensor_->dtype() == DataType::DT_INT32) {
        GetValueVec<int32, int64_t>(&values_vec);
      } else if (tensor_->dtype() == DataType::DT_INT64) {
        GetValueVec<int64_t, int64_t>(&values_vec);
      }
      BuildOutput<int64_t>(0, 0, values_vec, &tfprof_tensor_pb_);
      break;
    }
    case DataType::DT_STRING: {
      // Not supported by TensorFlow.
      std::vector<tstring> values_vec;
      GetValueVec<tstring, tstring>(&values_vec);
      BuildOutput<tstring>(0, 0, values_vec, &tfprof_tensor_pb_);
      break;
    }
    default: {
      absl::FPrintF(stderr, "Not Supported type %d\n", tensor_->dtype());
      break;
    }
  }
}

}  // namespace tfprof
}  // namespace tensorflow
