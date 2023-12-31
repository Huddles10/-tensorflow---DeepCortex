// RUN: hlo_to_kernel --input=%s --output=%t --unroll_factors=4 --tile_sizes=256 --arch=sm_70

func.func @tanh(%arg: tensor<*xf32>) -> tensor<*xf32> attributes {tf_entry} {
  %0 = mhlo.tanh %arg : tensor<*xf32>
  return %0 : tensor<*xf32>
}
