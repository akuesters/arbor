#pragma once

#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include <arbor/event_generator.hpp>
#include <arbor/cable_cell_param.hpp>
#include <arbor/recipe.hpp>

#include "error.hpp"
#include "strprintf.hpp"

namespace pyarb {

//struct probe_info_shim {
//    arb::cell_member_type id;
//    arb::probe_tag tag;
//
//    // Address type will be specific to cell kind of cell `id.gid`.
//    pybind11::object address;
//};

// pyarb::recipe is the recipe interface used by Python.
// Calls that return generic types return pybind11::object, to avoid
// having to wrap some C++ types used by the C++ interface (specifically
// util::unique_any, util::any, std::unique_ptr, etc.)
// For example, requests for cell description return pybind11::object, instead
// of util::unique_any used by the C++ recipe interface.
// The py_recipe_shim unwraps the python objects, and forwards them
// to the C++ back end.

class py_recipe {
public:
    py_recipe() = default;
    virtual ~py_recipe() {}

    virtual arb::cell_size_type num_cells() const = 0;
    virtual pybind11::object cell_description(arb::cell_gid_type gid) const = 0;
    virtual arb::cell_kind cell_kind(arb::cell_gid_type gid) const = 0;

    virtual arb::cell_size_type num_sources(arb::cell_gid_type) const {
        return 0;
    }
    virtual arb::cell_size_type num_targets(arb::cell_gid_type) const {
        return 0;
    }
    virtual arb::cell_size_type num_gap_junction_sites(arb::cell_gid_type gid) const {
        return gap_junctions_on(gid).size();
    }
    virtual std::vector<pybind11::object> event_generators(arb::cell_gid_type gid) const {
        return {};
    }
    virtual std::vector<arb::cell_connection> connections_on(arb::cell_gid_type gid) const {
        return {};
    }
    virtual std::vector<arb::gap_junction_connection> gap_junctions_on(arb::cell_gid_type) const {
        return {};
    }
    virtual arb::cell_size_type num_probes(arb::cell_gid_type) const {
        return 0;
    }
//    virtual probe_info_shim get_probe (arb::cell_member_type id) const {
    virtual arb::probe_info get_probe (arb::cell_member_type id) const {
//    virtual pybind11::object probe (arb::cell_member_type id) const {
        throw pyarb_error(util::pprintf("bad probe id {}", id));
    }
    //TODO: virtual pybind11::object get_probe (arb::cell_member_type id) const {...}
    //TODO: virtual pybind11::object global_properties(arb::cell_kind kind) const {return pybind11::none();};
};

class py_recipe_trampoline: public py_recipe {
public:
    arb::cell_size_type num_cells() const override {
        PYBIND11_OVERLOAD_PURE(arb::cell_size_type, py_recipe, num_cells);
    }

    pybind11::object cell_description(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD_PURE(pybind11::object, py_recipe, cell_description, gid);
    }

    arb::cell_kind cell_kind(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD_PURE(arb::cell_kind, py_recipe, cell_kind, gid);
    }

    arb::cell_size_type num_sources(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(arb::cell_size_type, py_recipe, num_sources, gid);
    }

    arb::cell_size_type num_targets(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(arb::cell_size_type, py_recipe, num_targets, gid);
    }

    arb::cell_size_type num_gap_junction_sites(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(arb::cell_size_type, py_recipe, num_gap_junction_sites, gid);
    }

    std::vector<pybind11::object> event_generators(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(std::vector<pybind11::object>, py_recipe, event_generators, gid);
    }

    std::vector<arb::cell_connection> connections_on(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(std::vector<arb::cell_connection>, py_recipe, connections_on, gid);
    }

    std::vector<arb::gap_junction_connection> gap_junctions_on(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(std::vector<arb::gap_junction_connection>, py_recipe, gap_junctions_on, gid);
    }

    arb::cell_size_type num_probes(arb::cell_gid_type gid) const override {
        PYBIND11_OVERLOAD(arb::cell_size_type, py_recipe, num_probes, gid);
    }

//    probe_info_shim get_probe(arb::cell_member_type id) const override {
//        PYBIND11_OVERLOAD(probe_info_shim, py_recipe, get_probe, id);
    arb::probe_info get_probe(arb::cell_member_type id) const override {
        PYBIND11_OVERLOAD(arb::probe_info, py_recipe, get_probe, id);
//    pybind11::object probe(arb::cell_member_type id) const override {
//        PYBIND11_OVERLOAD(pybind11::object, py_recipe, probe, id);
    }
    //TODO: pybind11::object get_probe(arb::cell_member_type id)
};

// A recipe shim that holds a pyarb::recipe implementation.
// Unwraps/translates python-side output from pyarb::recipe and forwards
// to arb::recipe.
// For example, unwrap cell descriptions stored in PyObject, and rewrap
// in util::unique_any.

class py_recipe_shim: public arb::recipe {
    // pointer to the python recipe implementation
    std::shared_ptr<py_recipe> impl_;

public:
    using recipe::recipe;

    py_recipe_shim(std::shared_ptr<py_recipe> r): impl_(std::move(r)) {}

    arb::cell_size_type num_cells() const override {
        return impl_->num_cells();
    }

    // The pyarb::recipe::cell_decription returns a pybind11::object, that is
    // unwrapped and copied into a util::unique_any.
    arb::util::unique_any get_cell_description(arb::cell_gid_type gid) const override;

    arb::cell_kind get_cell_kind(arb::cell_gid_type gid) const override {
        return impl_->cell_kind(gid);
    }

    arb::cell_size_type num_sources(arb::cell_gid_type gid) const override {
        return impl_->num_sources(gid);
    }

    arb::cell_size_type num_targets(arb::cell_gid_type gid) const override {
        return impl_->num_targets(gid);
    }

    arb::cell_size_type num_gap_junction_sites(arb::cell_gid_type gid) const override {
        return impl_->num_gap_junction_sites(gid);
    }

    std::vector<arb::event_generator> event_generators(arb::cell_gid_type gid) const override;

    std::vector<arb::cell_connection> connections_on(arb::cell_gid_type gid) const override {
        return impl_->connections_on(gid);
    }

    std::vector<arb::gap_junction_connection> gap_junctions_on(arb::cell_gid_type gid) const override {
        return impl_->gap_junctions_on(gid);
    }

    arb::cell_size_type num_probes(arb::cell_gid_type gid) const override {
        return impl_->num_probes(gid);
    }

    arb::probe_info get_probe(arb::cell_member_type id) const override {//;
        return impl_->get_probe(id);
    }
    //TODO: arb::probe_info get_probe(arb::cell_member_type id)

    // TODO: wrap
    arb::util::any get_global_properties(arb::cell_kind kind) const override {
        if (kind==arb::cell_kind::cable) {
            arb::cable_cell_global_properties gprop;
            gprop.default_parameters = arb::neuron_parameter_defaults;
            return gprop;
        }
        return arb::util::any{};
    }
};

} // namespace pyarb
