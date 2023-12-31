# Copyright 2018 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Tests for eager profiler."""

import os

from tensorflow.python.eager import profiler
from tensorflow.python.eager import test
from tensorflow.python.framework import config
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import test_util
from tensorflow.python.platform import gfile
from tensorflow.python.profiler import trace
from tsl.profiler.protobuf import trace_events_pb2


@test_util.with_eager_op_as_function
class ProfilerTest(test_util.TensorFlowTestCase):

  # TODO(b/201538012): Re-enable eager-op-as-funcition when trace discrepancies
  #                    have been fixed, ideally at trace level 2.
  @test_util.disable_eager_op_as_function
  def test_profile(self):
    profiler.start(options={'host_tracer_level': 2})
    with trace.Trace('three_times_five'):
      three = constant_op.constant(3)
      five = constant_op.constant(5)
      product = three * five
    self.assertAllEqual(15, product)
    with self.assertRaises(profiler.ProfilerAlreadyRunningError):
      profiler.start()

    profile_result = profiler.stop()
    profile_pb = trace_events_pb2.Trace()
    profile_pb.ParseFromString(profile_result)
    devices = frozenset(device.name for device in profile_pb.devices.values())
    self.assertIn('/host:CPU', devices)
    if config.list_physical_devices('GPU'):
      self.assertIn('/device:GPU:0', devices)
    events = frozenset(event.name for event in profile_pb.trace_events)
    self.assertIn('three_times_five', events)
    self.assertIn('Mul', events)
    with self.assertRaises(profiler.ProfilerNotRunningError):
      profiler.stop()

  def test_save_profile(self):
    logdir = self.get_temp_dir()
    profile_pb = trace_events_pb2.Trace()
    profile_result = profile_pb.SerializeToString()
    profiler.save(logdir, profile_result)
    file_list = gfile.ListDirectory(logdir)
    self.assertEqual(len(file_list), 2)
    for file_name in gfile.ListDirectory(logdir):
      if gfile.IsDirectory(os.path.join(logdir, file_name)):
        self.assertEqual(file_name, 'plugins')
      else:
        self.assertTrue(file_name.endswith('.profile-empty'))


if __name__ == '__main__':
  test.main()
