#include "pedersen.hpp"

using namespace proof_system::plonk::stdlib::types;

namespace acir_format {

void create_pedersen_constraint(Composer& composer, const PedersenConstraint& input)
{
    std::vector<field_ct> scalars;

    for (const auto& scalar : input.scalars) {
        // convert input indices to field_ct
        field_ct scalar_as_field = field_ct::from_witness_index(&composer, scalar);
        scalars.push_back(scalar_as_field);
    }
#ifdef USE_TURBO
    auto point = pedersen_commitment::commit(scalars);
#else
    // TODO: Does Noir need additive homomorphic Pedersen hash? If so, using plookup version won't help.
    auto point = stdlib::pedersen_plookup_commitment<Composer>::commit(scalars);
#endif

    composer.assert_equal(point.x.witness_index, input.result_x);
    composer.assert_equal(point.y.witness_index, input.result_y);
}

} // namespace acir_format
