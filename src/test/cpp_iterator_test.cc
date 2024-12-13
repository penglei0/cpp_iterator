#include "../cpp_iterator.h"

#include <gtest/gtest.h>

TEST(PacketTest, iterator_test) {
  Packet packet;
  int subframe_num = 2;
  int data_cell_num_for_subframe = 3;
  auto& data = packet.data();
  // FrameHeader into data
  struct FrameHeader frame_header = {1, 1, static_cast<uint16_t>(subframe_num)};
  data.insert(data.end(), reinterpret_cast<char*>(&frame_header),
              reinterpret_cast<char*>(&frame_header) + FRAME_HDR_LENGTH);
  for (int i = 0; i < subframe_num; i++) {
    // SubFrameHeader into data
    struct SubFrameHeader sub_frame_header = {2, 3, 1};
    sub_frame_header.len =
        static_cast<uint16_t>(data_cell_num_for_subframe * DATA_CELL_SIZE);
    std::cout << "Added subframe len: " << sub_frame_header.len << std::endl;
    data.insert(
        data.end(), reinterpret_cast<char*>(&sub_frame_header),
        reinterpret_cast<char*>(&sub_frame_header) + SUBFRAME_HDR_LENGTH);
    for (uint16_t j = 0; j < data_cell_num_for_subframe; j++) {
      // DataCell into data
      struct DataCell data_cell = {static_cast<uint16_t>(3 + j),
                                   static_cast<uint8_t>(4 + j)};
      data.insert(data.end(), reinterpret_cast<char*>(&data_cell),
                  reinterpret_cast<char*>(&data_cell) + DATA_CELL_SIZE);
      std::cout << "Added DataCell id: " << data_cell.id << std::endl;
    }
  }

  packet.Deserialize();

  for (auto subframe_iter = packet.begin(); subframe_iter != packet.end();
       ++subframe_iter) {
    int j = 0;
    for (auto data_cell_iter = subframe_iter.begin();
         data_cell_iter != subframe_iter.end(); ++data_cell_iter) {
      const DataCell* data_cell = *data_cell_iter;
      std::cout << "** Iterator DataCell id: " << data_cell->id << std::endl;
      EXPECT_EQ(data_cell->id, 3 + j);
      EXPECT_EQ(data_cell->symbol_num, 4 + j);
      j++;
    }
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
