#pragma once

#include <arbor/version.hpp>

#include <sstream>
#include <iomanip>
#include <ios>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pyarb {

// Create and return a dictionary
pybind11::dict config() {
    pybind11::dict dict;
    #ifdef ARB_MPI_ENABLED
        dict[pybind11::str("mpi")]     = pybind11::bool_(true);
    #else
        dict[pybind11::str("mpi")]     = pybind11::bool_(false);
    #endif
    #ifdef ARB_WITH_MPI4PY
        dict[pybind11::str("mpi4py")]  = pybind11::bool_(true);
    #else
        dict[pybind11::str("mpi4py")]  = pybind11::bool_(false);
    #endif
    #ifdef ARB_WITH_GPU
        dict[pybind11::str("gpu")]     = pybind11::bool_(true);
    #else
        dict[pybind11::str("gpu")]     = pybind11::bool_(false);
    #endif
        dict[pybind11::str("version")] = pybind11::str(ARB_VERSION);
    return dict;
}

void print_config(const pybind11::dict &d) {
    std::stringstream s;
    s << "Arbor's configuration:" << std::endl;

    for (auto x: d) {
        s << "     " << std::left << std::setw(7) << x.first << ": " << std::right << std::setw(10) << x.second << "\n";
    }

    pybind11::print(s.str());
}
} // namespace pyarb
