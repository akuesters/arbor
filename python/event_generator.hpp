#pragma once

#include <arbor/common_types.hpp>
#include <arbor/schedule.hpp>

#include <pybind11/pybind11.h>

namespace pyarb {

struct event_generator_shim {
    arb::cell_member_type target;
    double weight;
    arb::schedule time_sched;

    event_generator_shim(arb::cell_member_type cell, double event_weight, arb::schedule sched):
        target(cell),
        weight(event_weight),
        time_sched(std::move(sched))
    {}
};

template <typename Sched>
event_generator_shim make_event_generator(
        arb::cell_member_type target,
        double weight,
        const Sched& sched)
{
    return event_generator_shim(target, weight, sched.schedule());
}

} // namespace pyarb