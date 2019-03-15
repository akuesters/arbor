#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include <arbor/common_types.hpp>
#include <arbor/event_generator.hpp>
#include <arbor/schedule.hpp>

#include "event_generator.hpp"

namespace pyarb {

// ===================================== Helper ==========================================
// build an event generator given a lid, a weight, and a schedule shim.

template <typename Sched>
event_generator make_event_generator(
        arb::cell_lid_type lid,
        double weight,
        const Sched& sched)
{
    return event_generator(lid, weight, sched.schedule());
}

// ===================================== Register =====================================
void register_event_generators(pybind11::module& m) {
    using namespace pybind11::literals;

// Common schedules
    // Regular schedule
    pybind11::class_<regular_schedule_shim> regular_schedule(m, "regular_schedule",
        "Describes a regular schedule with multiples of dt within the interval [tstart, tstop).");

    regular_schedule
        .def(pybind11::init<>(),
            "Construct a regular schedule with default arguments:\n"
            "  tstart: maximal numerical number.\n"
            "  dt:     0 ms.\n"
            "  tstop:  tstart.")
        .def(pybind11::init<arb::time_type>(),
            "dt"_a,
            "Construct a regular schedule starting at tstart=0 ms, ending at tstop=maximal numerical number (in ms) with argument:\n"
            "  dt:      The distance between time points in the regular sequence (in ms).\n")
        .def(pybind11::init<arb::time_type, arb::time_type, arb::time_type>(),
            "tstart"_a, "tstop"_a, "dt"_a,
            "Construct a regular schedule with arguments:\n"
            "  tstart:  The first time in the regular sequence (in ms).\n"
            "  tstop:   The latest possible time in the regular sequence (in ms).\n"
            "  dt:      The distance between time points in the regular sequence (in ms).\n")
        .def_readwrite("tstart", &regular_schedule_shim::tstart,
            "The first time in the regular sequence (in ms).")
        .def_readwrite("tstop",  &regular_schedule_shim::tstop,
            "The latest possible time in the regular sequence (in ms).")
        .def_readwrite("dt",     &regular_schedule_shim::dt,
            "The distance between time points in the regular sequence (in ms).");

    // Explicit schedule
    pybind11::class_<explicit_schedule_shim> explicit_schedule(m, "explicit_schedule",
        "Describes an explicit schedule at times given explicitly via a provided sorted sequence.");

    explicit_schedule
        .def(pybind11::init<>(), "Construct an explicit schedule with an empty list of times.")
        .def(pybind11::init<pybind11::list>(),
            "times"_a,
            "Construct an explicit schedule with argument:\n"
            "  times: A list of times in the explicit schedule (in ms).")
        .def_readwrite("times", &explicit_schedule_shim::py_times,
            "A list of times in the explicit schedule (in ms).");

    // Poisson schedule
    pybind11::class_<poisson_schedule_shim> poisson_schedule(m, "poisson_schedule",
        "Describes a schedule at Poisson point process with rate 1/mean_dt, restricted to non-negative times.");

    poisson_schedule
        .def(pybind11::init<>(),
            "Construct a Poisson schedule with default arguments:\n"
            "  tstart: 0 ms.\n"
            "  freq:   10 Hz.\n"
            "  seed:   0 for a Mersenne Twister pseudo-random generator of 64-bit numbers with a state size of 19937 bits.")
        .def(pybind11::init<arb::time_type, std::mt19937_64::result_type>(),
            "Construct a Poisson schedule with arguments:\n"
            "  freq:   The frequency (in Hz).\n"
            "  seed:   The seed of the Mersenne Twister pseudo-random generator of 64-bit numbers with a state size of 19937 bits.")
        .def(pybind11::init<arb::time_type, arb::time_type, std::mt19937_64::result_type>(),
            "Construct a Poisson schedule with arguments:\n"
            "  tstart: The first time in the Poisson schedule (in ms).\n"
            "  freq:   The frequency (in Hz).\n"
            "  seed:   The seed of the Mersenne Twister pseudo-random generator of 64-bit numbers with a state size of 19937     bits.")
        .def_readwrite("tstart", &poisson_schedule_shim::tstart,
            "The first time in the Poisson schedule (in ms).")
        .def_readwrite("freq", &poisson_schedule_shim::freq,
            "The frequency (in Hz).")
        .def_readwrite("seed", &poisson_schedule_shim::seed,
            "The seed of the Mersenne Twister pseudo-random generator of 64-bit numbers with a state size of 19937 bits.");

// Event generator
    pybind11::class_<event_generator> event_generator(m, "event_generator");

    event_generator
        .def(pybind11::init<>(
            [](arb::cell_lid_type lid, double weight, const regular_schedule_shim& sched){
                return make_event_generator(lid, weight, sched);}))
        .def(pybind11::init<>(
            [](arb::cell_lid_type lid, double weight, const explicit_schedule_shim& sched){
                return make_event_generator(lid, weight, sched);}))
        .def(pybind11::init<>(
            [](arb::cell_lid_type lid, double weight, const poisson_schedule_shim& sched){
                return make_event_generator(lid, weight, sched);}));
}

} // namespace pyarb
