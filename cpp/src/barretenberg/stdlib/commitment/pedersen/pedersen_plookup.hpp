#pragma once
#include "../../primitives/composers/composers_fwd.hpp"
#include "../../primitives/field/field.hpp"
#include "../../primitives/point/point.hpp"
#include "../../primitives/packed_byte_array/packed_byte_array.hpp"

namespace proof_system::plonk {
namespace stdlib {

template <typename ComposerContext> class pedersen_plookup_commitment {
  private:
    typedef stdlib::field_t<ComposerContext> field_t;
    typedef stdlib::point<ComposerContext> point;
    typedef stdlib::packed_byte_array<ComposerContext> packed_byte_array;
    typedef stdlib::bool_t<ComposerContext> bool_t;

  public:
    static point commit(const std::vector<field_t>& inputs, const size_t hash_index = 0);
    static point commit(const std::vector<field_t>& inputs, const std::vector<size_t>& hash_indices);

    static field_t compress(const field_t& left, const field_t& right);
    static field_t compress(const std::vector<field_t>& inputs, const size_t hash_index = 0);
    static field_t compress(const packed_byte_array& input) { return compress(input.get_limbs()); }

    static field_t compress(const std::vector<field_t>& inputs, const std::vector<size_t>& hash_indices);
    static field_t compress(const std::vector<std::pair<field_t, size_t>>& input_pairs);

    template <size_t T> static field_t compress(const std::array<field_t, T>& inputs)
    {
        std::vector<field_t> in(inputs.begin(), inputs.end());
        return compress(in);
    }

    static point merkle_damgard_compress(const std::vector<field_t>& inputs, const field_t& iv);
    static point merkle_damgard_compress(const std::vector<field_t>& inputs, const std::vector<field_t>& ivs);
    static point merkle_damgard_tree_compress(const std::vector<field_t>& inputs, const std::vector<field_t>& ivs);

    static point compress_to_point(const field_t& left, const field_t& right);
};

extern template class pedersen_plookup_commitment<UltraComposer>;
} // namespace stdlib
} // namespace proof_system::plonk