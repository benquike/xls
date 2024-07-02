// Copyright 2021 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xls/examples/jpeg/jpeg_grm.h"

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "xls/common/status/matchers.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/streams.h"
#include "xls/examples/jpeg/streams.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"
#include "xls/examples/jpeg/constants.h"

namespace xls::jpeg {
namespace {

TEST(JpegGrm, SimpleRgbConversions) {
  const Rgb black = {0, 0, 0};
  EXPECT_EQ(internal::ToRgb(0, kChrominanceZero, kChrominanceZero), black);
  const Rgb white = {255, 255, 255};
  EXPECT_EQ(internal::ToRgb(255, kChrominanceZero, kChrominanceZero), white);
}

TEST(JpegGrm, HuffmanTableExpandEntriesDcOneEntry) {
  internal::HuffmanTable table{
      .table_class = internal::HuffmanTableClass::kDc,
      .table_index = 0,
  };
  // We create a single entry in the "1 bit code" bucket.
  // In the DC table we should never have leading zero values.
  // We make the entry say we should pop 3 bits out of the bit stream.
  table.entries[0].push_back(0 << 4 | 3);
  XLS_ASSERT_OK(internal::HuffmanTableExpandEntries(&table));
  EXPECT_EQ(table.expanded.size(), 1);
  EXPECT_EQ(table.expanded[0].bits, 1);
  EXPECT_EQ(table.expanded[0].GetBitsToPop(), 3);
  EXPECT_EQ(table.expanded[0].GetLeadingZeros(), 0);

  {
    const internal::PrefixEntry* entry =
        MatchLookahead(table, /*lookahead=*/0 << 15);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry, &table.expanded[0]);
    EXPECT_EQ(entry->code, 0);
  }

  {
    const internal::PrefixEntry* entry =
        MatchLookahead(table, /*lookahead=*/1 << 15);
    ASSERT_EQ(entry, nullptr);
  }
}

TEST(JpegGrm, HuffmanTableExpandEntriesAcTwoEntries) {
  internal::HuffmanTable table{
      .table_class = internal::HuffmanTableClass::kAc,
      .table_index = 0,
  };
  // Create one entry in the "2 bit" bucket with 5 leading zeros and 3 bits to
  // pop.
  table.entries[1].push_back(5 << 4 | 3);
  // Create another entry in the "4 bit" bucket with 1 leading zero and 7 bits
  // to pop.
  table.entries[3].push_back(1 << 4 | 7);
  XLS_ASSERT_OK(internal::HuffmanTableExpandEntries(&table));
  EXPECT_EQ(table.expanded.size(), 2);
  EXPECT_EQ(table.expanded[0].bits, 2);
  EXPECT_EQ(table.expanded[0].GetBitsToPop(), 3);
  EXPECT_EQ(table.expanded[0].GetLeadingZeros(), 5);
  EXPECT_EQ(table.expanded[0].code, 0);

  EXPECT_EQ(table.expanded[1].bits, 4);
  EXPECT_EQ(table.expanded[1].GetBitsToPop(), 7);
  EXPECT_EQ(table.expanded[1].GetLeadingZeros(), 1);
  EXPECT_EQ(table.expanded[1].code, 4);
}

TEST(JpegGrm, OneMcuAllWhiteImage) {
  // Sample bytes generated via:
  // ```python
  // from PIL import Image
  // image = Image.new(mode='RGB', size=(8, 8), color='white')
  // image.save(path, quality=1, subsampling=0)
  // ```
  //
  // Trying to make a small image to use as a smoke test.
  const std::vector<uint8_t> data = {
      0xff, 0xd8,  // Start of Image marker
      0xff, 0xe0,  // App0 marker
      0x0,  0x10, 0x4a, 0x46, 0x49, 0x46, 0x0,  0x1,  0x1,  0x0,  0x0,  0x1,
      0x0,  0x1,  0x0,  0x0,  //
      0xff, 0xdb,             // Define Quantization Table(s) marker
      0x0,  0x43, 0x0,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //
      0xff, 0xdb,  // Define Quantization Table(s) marker
      0x0,  0x43, 0x1,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0,  0x11, 0x8,
      0x0,  0x8,  0x0,  0x8,  0x3,  0x1,  0x11, 0x0,  0x2,  0x11, 0x1,  0x3,
      0x11, 0x1,  0xff, 0xc4, 0x0,  0x1f, 0x0,  0x0,  0x1,  0x5,  0x1,  0x1,
      0x1,  0x1,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
      0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,  0x9,  0xa,  0xb,  //
      0xff, 0xc4,  // Define Huffman Table(s) marker
      0x0,  0xb5, 0x10, 0x0,  0x2,  0x1,  0x3,  0x3,  0x2,  0x4,  0x3,  0x5,
      0x5,  0x4,  0x4,  0x0,  0x0,  0x1,  0x7d, 0x1,  0x2,  0x3,  0x0,  0x4,
      0x11, 0x5,  0x12, 0x21, 0x31, 0x41, 0x6,  0x13, 0x51, 0x61, 0x7,  0x22,
      0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x8,  0x23, 0x42, 0xb1, 0xc1, 0x15,
      0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x9,  0xa,  0x16, 0x17,
      0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
      0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
      0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
      0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
      0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
      0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
      0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
      0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
      0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
      0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa,        //
      0xff, 0xc4,  // Define Huffman Table(s) marker
      0x0,  0x1f, 0x1,  0x0,  0x3,  0x1,  0x1,  0x1,  0x1,  0x1,  0x1,  0x1,
      0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x2,  0x3,  0x4,
      0x5,  0x6,  0x7,  0x8,  0x9,  0xa,  0xb,  //
      0xff, 0xc4,  // Define Huffman Table(s) marker
      0x0,  0xb5, 0x11, 0x0,  0x2,  0x1,  0x2,  0x4,  0x4,  0x3,  0x4,  0x7,
      0x5,  0x4,  0x4,  0x0,  0x1,  0x2,  0x77, 0x0,  0x1,  0x2,  0x3,  0x11,
      0x4,  0x5,  0x21, 0x31, 0x6,  0x12, 0x41, 0x51, 0x7,  0x61, 0x71, 0x13,
      0x22, 0x32, 0x81, 0x8,  0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x9,  0x23,
      0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0xa,  0x16, 0x24, 0x34, 0xe1,
      0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35,
      0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
      0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65,
      0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
      0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93,
      0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
      0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
      0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3,
      0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
      0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa,        //
      0xff, 0xda,  // Start of Scan marker
      0x0,  0xc,  0x3,  0x1,  0x0,  0x2,  0x11, 0x3,  0x11, 0x0,  0x3f, 0x0,
      0x92, 0x80, 0x3f,  //
      0xff, 0xd9         // End of Image marker
  };
  SpanPopper popper(data);
  ByteStream byte_stream(popper);
  XLS_ASSERT_OK_AND_ASSIGN(DecodedJpeg jpeg, DecodeJpeg(byte_stream));
  EXPECT_EQ(jpeg.y, kMcuHeight);
  EXPECT_EQ(jpeg.x, kMcuWidth);
  EXPECT_EQ(jpeg.GetPixelCount(), kCoeffPerMcu);
  EXPECT_EQ(jpeg.GetPixelCountViaLineData(), kCoeffPerMcu);
  for (int y = 0; y < jpeg.y; ++y) {
    for (int x = 0; x < jpeg.x; ++x) {
      const Rgb& rgb = jpeg.lines[y][x];
      VLOG(1) << absl::StreamFormat("(y, x) = (%d, %d) => ", y, x) << rgb;
      ASSERT_EQ(rgb.r, 255);
      ASSERT_EQ(rgb.g, 255);
      ASSERT_EQ(rgb.b, 255);
    }
  }
}

}  // namespace
}  // namespace xls::jpeg
