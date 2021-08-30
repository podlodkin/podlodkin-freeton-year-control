#define BOOST_TEST_MODULE snark_logic_test
#include <boost/test/included/unit_test.hpp>

#include "../bin/comparerLogic.h"

using namespace nil::crypto3;
using namespace nil::crypto3::zk;

template<typename FieldType>
bool test_bp_input(int minYear, int maxYear, int year) {
    blueprint<FieldType> bp;
    ComparerLogic<FieldType> comparerLogic(bp);
    comparerLogic.generate_r1cs_constraints(bp);
    comparerLogic.generate_r1cs_witness(bp, minYear, maxYear, year);
    return bp.is_satisfied();
}

void test_comparerLogic_range(bool isSatisfied, int minYear, int maxYear, int year) {
    std::cout << "Testing " << year <<" in range (" << minYear << ", " << maxYear << ")" << std::endl;
    BOOST_CHECK(test_bp_input<field_type>(minYear, maxYear, year) == isSatisfied);
}


BOOST_AUTO_TEST_SUITE(comparerLogic_test_suite)

int minYear = 2,
    maxYear = 100,
    yearInside = 5,
    yearOutsideLess = 1,
    yearOutsideMore = 101;

BOOST_AUTO_TEST_CASE(comparerLogic_test_valid_ranges) {
    test_comparerLogic_range(true, minYear, maxYear, yearInside);
}

BOOST_AUTO_TEST_CASE(comparerLogic_test_invalid_ranges) {
    test_comparerLogic_range(false, minYear, maxYear, yearOutsideLess);
    test_comparerLogic_range(false, minYear, maxYear, yearOutsideMore);
}

BOOST_AUTO_TEST_SUITE_END()
