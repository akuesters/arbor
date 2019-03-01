#pragma once

/*
 * Utilities for generating string representations of types.
 */

#include <string>

#include <arbor/common_types.hpp>
#include <arbor/context.hpp>
#include <arbor/recipe.hpp>

namespace pyarb {

std::string cell_member_string(const arb::cell_member_type&);
std::string connection_string(const arb::cell_connection&);
std::string context_string(const arb::context&);
std::string gap_junction_connection_string(const arb::gap_junction_connection&);
std::string proc_allocation_string(const arb::proc_allocation&);

} // namespace pyarb
