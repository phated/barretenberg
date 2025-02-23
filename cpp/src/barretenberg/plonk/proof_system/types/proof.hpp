#pragma once
#include <cstdint>
#include <vector>
#include <ostream>
#include <iomanip>
#include "barretenberg/common/serialize.hpp"

namespace proof_system::plonk {

struct proof {
    std::vector<uint8_t> proof_data;

    bool operator==(proof const& other) const = default;
};

inline void read(uint8_t const*& it, proof& data)
{
    using serialize::read;
    read(it, data.proof_data);
};

template <typename B> inline void write(B& buf, proof const& data)
{
    using serialize::write;
    write(buf, data.proof_data);
}

inline std::ostream& operator<<(std::ostream& os, proof const& data)
{
    // REFACTOR: This is copied from barretenberg/common/streams.hpp,
    // which means we could just cout proof_data directly, but that breaks the build in the CI with 
    // a redefined operator<< error in barretenberg/stdlib/hash/keccak/keccak.test.cpp,
    // which is something we really don't want to deal with right now.
    std::ios_base::fmtflags f(os.flags());
    os << "[" << std::hex << std::setfill('0');
    for (auto byte : data.proof_data) {
        os << ' ' << std::setw(2) << +(unsigned char)byte;
    }
    os << " ]";
    os.flags(f);
    return os;
}

} // namespace proof_system::plonk
