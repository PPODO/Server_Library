// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_WATCHDOGPING_WATCHDOGPACKET_H_
#define FLATBUFFERS_GENERATED_WATCHDOGPING_WATCHDOGPACKET_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 25 &&
              FLATBUFFERS_VERSION_MINOR == 2 &&
              FLATBUFFERS_VERSION_REVISION == 10,
             "Non-compatible flatbuffers version included");

namespace WatchDogPacket {

struct PingPacket;
struct PingPacketBuilder;

struct PingPacket FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef PingPacketBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CPU_USAGE = 4,
    VT_MEMORY_USAGE = 6,
    VT_TOTAL_MEMORY = 8,
    VT_CONNECTED_USER_COUNT = 10
  };
  float cpu_usage() const {
    return GetField<float>(VT_CPU_USAGE, 0.0f);
  }
  double memory_usage() const {
    return GetField<double>(VT_MEMORY_USAGE, 0.0);
  }
  uint64_t total_memory() const {
    return GetField<uint64_t>(VT_TOTAL_MEMORY, 0);
  }
  int32_t connected_user_count() const {
    return GetField<int32_t>(VT_CONNECTED_USER_COUNT, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_CPU_USAGE, 4) &&
           VerifyField<double>(verifier, VT_MEMORY_USAGE, 8) &&
           VerifyField<uint64_t>(verifier, VT_TOTAL_MEMORY, 8) &&
           VerifyField<int32_t>(verifier, VT_CONNECTED_USER_COUNT, 4) &&
           verifier.EndTable();
  }
};

struct PingPacketBuilder {
  typedef PingPacket Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_cpu_usage(float cpu_usage) {
    fbb_.AddElement<float>(PingPacket::VT_CPU_USAGE, cpu_usage, 0.0f);
  }
  void add_memory_usage(double memory_usage) {
    fbb_.AddElement<double>(PingPacket::VT_MEMORY_USAGE, memory_usage, 0.0);
  }
  void add_total_memory(uint64_t total_memory) {
    fbb_.AddElement<uint64_t>(PingPacket::VT_TOTAL_MEMORY, total_memory, 0);
  }
  void add_connected_user_count(int32_t connected_user_count) {
    fbb_.AddElement<int32_t>(PingPacket::VT_CONNECTED_USER_COUNT, connected_user_count, 0);
  }
  explicit PingPacketBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<PingPacket> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<PingPacket>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<PingPacket> CreatePingPacket(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float cpu_usage = 0.0f,
    double memory_usage = 0.0,
    uint64_t total_memory = 0,
    int32_t connected_user_count = 0) {
  PingPacketBuilder builder_(_fbb);
  builder_.add_total_memory(total_memory);
  builder_.add_memory_usage(memory_usage);
  builder_.add_connected_user_count(connected_user_count);
  builder_.add_cpu_usage(cpu_usage);
  return builder_.Finish();
}

inline const WatchDogPacket::PingPacket *GetPingPacket(const void *buf) {
  return ::flatbuffers::GetRoot<WatchDogPacket::PingPacket>(buf);
}

inline const WatchDogPacket::PingPacket *GetSizePrefixedPingPacket(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<WatchDogPacket::PingPacket>(buf);
}

inline bool VerifyPingPacketBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<WatchDogPacket::PingPacket>(nullptr);
}

inline bool VerifySizePrefixedPingPacketBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<WatchDogPacket::PingPacket>(nullptr);
}

inline void FinishPingPacketBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<WatchDogPacket::PingPacket> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPingPacketBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<WatchDogPacket::PingPacket> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace WatchDogPacket

#endif  // FLATBUFFERS_GENERATED_WATCHDOGPING_WATCHDOGPACKET_H_
