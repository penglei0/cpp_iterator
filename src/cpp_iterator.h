/**
 * @file cpp_iterator.h
 * @author Lei Peng (peng.lei@n-hop.com)
 * @brief Just a simple example of showing how to define/implement a customized
 * iterator.
 * @version 3.6.0
 * @date 2024-12-13
 *
 *
 */
#ifndef SRC_CPP_ITERATOR_H_
#define SRC_CPP_ITERATOR_H_
#include <stdint.h>

#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
/** Packet memory layout:
 * |FrameHeader | sub-frame 0 | sub-frame 1 | .. | sub-frame n |
 *
 *  sub-frame memory layout:
 * | SubFrameHeader | DataCell 0 | DataCell 1 | .. | DataCell n |
 */
struct FrameHeader {
  uint32_t id;
  uint16_t type : 1;
  uint16_t sub_frame_num : 15;
} __attribute__((__packed__));
#define FRAME_HDR_LENGTH sizeof(struct FrameHeader)

/// @brief SubFrameHeader under FrameHeader
struct SubFrameHeader {
  uint32_t id;
  uint16_t len : 14;    // the length of the subframe(not include the
                        // `SubFrameHeader`)
  uint16_t status : 2;  // 0,1,2
} __attribute__((__packed__));
#define SUBFRAME_HDR_LENGTH sizeof(struct SubFrameHeader)

/// @brief DataCell under SubFrameHeader
struct DataCell {
  uint16_t id;                  // The id of data cell
  uint8_t symbol_num;           // Number of symbols
} __attribute__((__packed__));  // make sure the size of DataCell is 3 bytes

constexpr int DATA_CELL_SIZE = sizeof(struct DataCell);

constexpr int max_subframe_num_in_one_packet = 128;
constexpr int max_data_cell_num_in_one_subframe = 32;

using SubFrameHeaderPtr = const struct SubFrameHeader *;
using DataCellPtr = const struct DataCell *;
using DataCellArray =
    std::array<DataCellPtr, max_data_cell_num_in_one_subframe>;
using SubFrameArray = std::array<std::pair<SubFrameHeaderPtr, DataCellArray>,
                                 max_subframe_num_in_one_packet>;

class ConstDataCellIter {
 public:
  ConstDataCellIter(const DataCellArray &batch_stat_array, uint16_t index)
      : data_cell_array_(batch_stat_array), index_(index) {}
  /// @brief Iterator to the next ConstDataCellIter
  ConstDataCellIter &operator++() {
    ++index_;
    return *this;
  }
  const DataCell *operator*() const { return data_cell_array_[index_]; }
  bool operator!=(const ConstDataCellIter &other) const {
    return index_ != other.index_;
  }

 protected:
  const DataCellArray &data_cell_array_;
  int index_ = 0;
};

class ConstSubframeIter {
 public:
  ConstSubframeIter(const SubFrameArray &task_slide_array, uint16_t index)
      : subframe_array_(task_slide_array), index_(index) {}
  /// @brief Iterator to the next SubFrameHeader
  ConstSubframeIter &operator++() {
    ++index_;
    return *this;
  }
  const SubFrameHeader *operator*() const {
    return subframe_array_[index_].first;
  }
  bool operator!=(const ConstSubframeIter &other) const {
    return index_ != other.index_;
  }

  /// @brief Iterator to the first DataCell of the SubFrameHeader
  ConstDataCellIter begin() const {
    return ConstDataCellIter(subframe_array_[index_].second, 0);
  }
  /// @brief Iterator to the last DataCell of the SubFrameHeader
  ConstDataCellIter end() const {
    return ConstDataCellIter(
        subframe_array_[index_].second,
        (subframe_array_[index_].first->len) / DATA_CELL_SIZE);
  }

 protected:
  const SubFrameArray &subframe_array_;
  int index_ = 0;
};

class Packet {
 public:
  Packet() = default;
  Packet &operator=(const Packet &input) = delete;
  Packet &operator=(Packet &&input) noexcept = delete;
  Packet(const Packet &input) = delete;
  Packet(Packet &&input) noexcept = delete;

  ConstSubframeIter begin() const {
    return ConstSubframeIter(subframe_array_, 0);
  }
  ConstSubframeIter end() const {
    return ConstSubframeIter(subframe_array_, subframe_num_);
  }
  const std::vector<char> &data() const { return data_; }
  std::vector<char> &data() { return data_; }

  // save all the pointers of SubFrameHeader and DataCell
  // into subframe_array_
  void Deserialize() {
    const char *ptr = data_.data();
    const char *end = data_.data() + data_.size();
    if (end - ptr < static_cast<int>(FRAME_HDR_LENGTH)) {
      std::cout << "Insufficient data to deserialize flow slide message."
                << std::endl;
      return;
    }
    // 1. parse FrameHeader
    const struct FrameHeader *msg_hdr =
        reinterpret_cast<const struct FrameHeader *>(ptr);
    subframe_num_ = static_cast<uint16_t>(msg_hdr->sub_frame_num);
    std::cout << "subframe_num_: " << subframe_num_ << std::endl;
    ptr += FRAME_HDR_LENGTH;
    if (subframe_num_ == 0) {
      return;
    }
    // 2. parse SubFrameHeader & DataCell
    assert(subframe_num_ <= max_subframe_num_in_one_packet);
    for (int i = 0; i < subframe_num_; i++) {
      if (end - ptr < static_cast<int>(SUBFRAME_HDR_LENGTH)) {
        std::cout << "Insufficient data to deserialize task slide message."
                  << std::endl;
        break;
      }
      // 2.1 parse SubFrameHeader
      const struct SubFrameHeader *subframe_hdr =
          reinterpret_cast<const struct SubFrameHeader *>(ptr);
      subframe_array_[i].first = subframe_hdr;
      ptr += SUBFRAME_HDR_LENGTH;
      if (end - ptr < static_cast<int>(DATA_CELL_SIZE)) {
        std::cout << "Insufficient data to deserialize batch stat."
                  << std::endl;
        break;
      }
      int len = subframe_hdr->len;
      std::cout << "len: " << len << std::endl;
      if (len % DATA_CELL_SIZE != 0) {
        std::cout << "Invalid task length." << std::endl;
        break;
      }
      int batch_num = len / DATA_CELL_SIZE;
      // 2.2 parse DataCell belongs to this SubFrameHeader
      assert(subframe_num_ <= max_subframe_num_in_one_packet);
      for (int j = 0; j < batch_num; j++) {
        subframe_array_[i].second[j] =
            reinterpret_cast<const struct DataCell *>(ptr);
        ptr += DATA_CELL_SIZE;
        std::cout << "DataCell id: " << subframe_array_[i].second[j]->id
                  << std::endl;
      }
    }
    std::cout << "Deserialize success." << std::endl;
  }

 private:
  std::vector<char> data_;
  SubFrameArray subframe_array_;
  int subframe_num_ = 0;
};

using Packet_ptr = std::shared_ptr<Packet>;

#endif  // SRC_CPP_ITERATOR_H_
