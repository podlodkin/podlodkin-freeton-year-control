#include <nil/crypto3/algebra/curves/bls12.hpp>
#include <nil/crypto3/algebra/fields/bls12/base_field.hpp>
#include <nil/crypto3/algebra/fields/bls12/scalar_field.hpp>
#include <nil/crypto3/algebra/fields/arithmetic_params/bls12.hpp>
#include <nil/crypto3/algebra/curves/params/multiexp/bls12.hpp>
#include <nil/crypto3/algebra/curves/params/wnaf/bls12.hpp>
#include <nil/crypto3/zk/components/blueprint.hpp>
#include <nil/crypto3/zk/components/blueprint_variable.hpp>
#include <nil/crypto3/zk/components/comparison.hpp>
#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark.hpp>

typedef algebra::curves::bls12<381> curve_type;
typedef curve_type::scalar_field_type field_type;
typedef field_type::value_type value_type;

typedef zk::snark::r1cs_gg_ppzksnark<curve_type> scheme_type;

using namespace std;
using namespace nil::crypto3::zk::components;
using namespace nil::crypto3::zk::snark;

template<typename field_type>
class ComparerLogic {

    public:
        blueprint_variable<field_type> minYear, maxYear, year;
        blueprint_variable<field_type> out;

        blueprint_variable<field_type> minYearLess, minYearLessOrEq, maxYearLess, maxYearLessOrEq;

        std::shared_ptr<comparison<field_type>> minYearCmp, maxYearCmp;//, resultCmp;

    ComparerLogic(blueprint<field_type> &bp) {
        minYear.allocate(bp);
        maxYear.allocate(bp);

        out.allocate(bp);

        year.allocate(bp);

        minYearLess.allocate(bp);
        maxYearLess.allocate(bp);
        minYearLessOrEq.allocate(bp);
        maxYearLessOrEq.allocate(bp);

        bp.set_input_sizes(2);
    }

    void generate_r1cs_constraints(blueprint<field_type> &bp) {
        std::size_t comparison_n = 50;

        minYearCmp.reset(new comparison<field_type>(bp, comparison_n, minYear, year, minYearLess, minYearLessOrEq));
        minYearCmp.get()->generate_r1cs_constraints();

        maxYearCmp.reset(new comparison<field_type>(bp, comparison_n, year, maxYear, maxYearLess, maxYearLessOrEq));
        maxYearCmp.get()->generate_r1cs_constraints();

        bp.add_r1cs_constraint(r1cs_constraint<field_type>(minYearLessOrEq + maxYearLessOrEq, 1, out));
    }

    void generate_r1cs_witness(blueprint<field_type> &bp, int minYear_, int maxYear_, int year_) {
        bp.val(minYear) = minYear_;
        bp.val(maxYear) = maxYear_;
        bp.val(year) = year_;

        bp.val(out) = 2;

        minYearCmp.get()->generate_r1cs_witness();
        maxYearCmp.get()->generate_r1cs_witness();
    }
};
