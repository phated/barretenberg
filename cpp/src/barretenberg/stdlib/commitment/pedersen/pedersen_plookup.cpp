#include "pedersen_plookup.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "../../hash/pedersen/pedersen_plookup.hpp"

#include "barretenberg/plonk/composer/plookup_tables/types.hpp"
#include "../../primitives/composers/composers.hpp"
#include "../../primitives/plookup/plookup.hpp"

namespace proof_system::plonk {
namespace stdlib {

using namespace barretenberg;

template <typename C>
point<C> pedersen_plookup_commitment<C>::compress_to_point(const field_t& left, const field_t& right)
{
    auto p2 = pedersen_plookup_hash<C>::hash_single(left, false);
    auto p1 = pedersen_plookup_hash<C>::hash_single(right, true);

    return pedersen_plookup_hash<C>::add_points(p1, p2);
}

template <typename C> field_t<C> pedersen_plookup_commitment<C>::compress(const field_t& left, const field_t& right)
{
    return compress_to_point(left, right).x;
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::merkle_damgard_compress(const std::vector<field_t>& inputs, const field_t& iv)
{
    if (inputs.size() == 0) {
        return point{ 0, 0 };
    }

    auto result = plookup_read::get_lookup_accumulators(MultiTableId::PEDERSEN_IV, iv)[ColumnIdx::C2][0];
    auto num_inputs = inputs.size();
    for (size_t i = 0; i < num_inputs; i++) {
        result = compress(result, inputs[i]);
    }

    return compress_to_point(result, field_t(num_inputs));
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::merkle_damgard_compress(const std::vector<field_t>& inputs,
                                                                 const std::vector<field_t>& ivs)
{
    const size_t num_inputs = inputs.size();
    if (num_inputs == 0) {
        return point{ 0, 0 };
    }

    auto result = plookup_read::get_lookup_accumulators(MultiTableId::PEDERSEN_IV, 0)[ColumnIdx::C2][0];
    for (size_t i = 0; i < 2 * num_inputs; i++) {
        if ((i & 1) == 0) {
            auto iv_result =
                plookup_read::get_lookup_accumulators(MultiTableId::PEDERSEN_IV, ivs[i >> 1])[ColumnIdx::C2][0];
            result = compress(result, iv_result);
        } else {
            result = compress(result, inputs[i >> 1]);
        }
    }

    return compress_to_point(result, field_t(num_inputs));
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::merkle_damgard_tree_compress(const std::vector<field_t>& inputs,
                                                                      const std::vector<field_t>& ivs)
{
    const size_t num_inputs = inputs.size();
    ASSERT(num_inputs == ivs.size());
    ASSERT(numeric::is_power_of_two(num_inputs));
    if (num_inputs == 0) {
        return point{ 0, 0 };
    }

    // Process height 0 of the tree.
    std::vector<field_t> temp_storage;
    for (size_t i = 0; i < num_inputs; i++) {
        auto iv_result = plookup_read::get_lookup_accumulators(MultiTableId::PEDERSEN_IV, ivs[i])[ColumnIdx::C2][0];
        temp_storage.push_back(compress(iv_result, inputs[i]));
    }

    // Process heights 1, 2, ..., log2(m) of the tree.
    const size_t total_height = numeric::get_msb(num_inputs);
    for (size_t height = 1; height <= total_height; height++) {
        const size_t leaf_count = 1UL << (total_height - height);
        for (size_t i = 0; i < leaf_count; i++) {
            temp_storage[i] = compress(temp_storage[2 * i], temp_storage[2 * i + 1]);
        }
    }

    return compress_to_point(temp_storage[0], field_t(num_inputs));
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::commit(const std::vector<field_t>& inputs, const size_t hash_index)
{
    return merkle_damgard_compress(inputs, field_t(hash_index));
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::commit(const std::vector<field_t>& inputs,
                                                const std::vector<size_t>& hash_indices)
{
    std::vector<field_t> hash_indices_;
    for (size_t i = 0; i < hash_indices.size(); i++) {
        hash_indices_.push_back(field_t(hash_indices[i]));
    }

    return merkle_damgard_compress(inputs, hash_indices_);
}

template <typename C>
field_t<C> pedersen_plookup_commitment<C>::compress(const std::vector<field_t>& inputs, const size_t hash_index)
{
    return commit(inputs, hash_index).x;
}

template <typename C>
field_t<C> pedersen_plookup_commitment<C>::compress(const std::vector<field_t>& inputs,
                                                    const std::vector<size_t>& hash_indices)
{
    return commit(inputs, hash_indices).x;
}

template <typename C>
field_t<C> pedersen_plookup_commitment<C>::compress(const std::vector<std::pair<field_t, size_t>>& input_pairs)
{
    std::vector<field_t> inputs;
    std::vector<size_t> hash_indices;
    for (size_t i = 0; i < input_pairs.size(); i++) {
        inputs.push_back(input_pairs[0].first);
        hash_indices.push_back(input_pairs[0].second);
    }

    return commit(inputs, hash_indices).x;
}

template class pedersen_plookup_commitment<UltraComposer>;

} // namespace stdlib
} // namespace proof_system::plonk