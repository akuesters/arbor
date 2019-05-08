#include <iostream>
#include <sstream>
#include <string>

#include <arbor/context.hpp>
#include <arbor/version.hpp>

#include <pybind11/pybind11.h>

#include "context.hpp"
#include "conversion.hpp"
#include "strings.hpp"

#ifdef ARB_MPI_ENABLED
#include "mpi.hpp"
#endif

namespace pyarb {

namespace {
auto is_nonneg_int = [](auto&& t){ return ((t==int(t) and t>=0)); };
}

void register_contexts(pybind11::module& m) {
    using namespace std::string_literals;
    using namespace pybind11::literals;
    using opt_int = arb::util::optional<int>;
#ifdef ARB_MPI_ENABLED
    using opt_mpi_comm = arb::util::optional<mpi_comm_shim>;
#endif

    pybind11::class_<arb::proc_allocation> proc_allocation(m, "proc_allocation");
    proc_allocation
        .def(pybind11::init(
            [](int threads, pybind11::object gpu){
                opt_int gpu_id = py2optional<int>(gpu,
                        "gpu must be None, or a non-negative integer.", is_nonneg_int);
                return arb::proc_allocation(threads, gpu_id.value_or(-1));
            }),
            "threads"_a=1, "gpu"_a=pybind11::none(),
            "Construct an allocation with arguments:\n"
            "  threads: The number of threads available locally for execution (defaults to 1).\n"
            "  gpu:     The index of the GPU to use (defaults to None for no GPU).\n")
        .def_readwrite("threads", &arb::proc_allocation::num_threads,
            "The number of threads available locally for execution.")
        .def_readwrite("gpu_id", &arb::proc_allocation::gpu_id,
            "The identifier of the GPU to use.\n"
            "Corresponds to the integer index used to identify GPUs in CUDA API calls.")
        .def_property_readonly("has_gpu", &arb::proc_allocation::has_gpu,
            "Whether a GPU is being used (True/False).")
        .def("__str__", &proc_allocation_string)
        .def("__repr__", &proc_allocation_string);

    pybind11::class_<context_shim> context(m, "context");
    context
        .def(pybind11::init<>(
            [](){return context_shim(arb::make_context());}),
            "Construct a local context with one thread, no, GPU, no MPI by default.\n"
            )
        .def(pybind11::init(
            [](const arb::proc_allocation& alloc){
                return context_shim(arb::make_context(alloc));
            }),
             "alloc"_a,
             "Construct a local context with argument:\n"
             "  alloc:   The computational resources to be used for the simulation.\n")
#ifdef ARB_MPI_ENABLED
        .def(pybind11::init(
            [](const arb::proc_allocation& alloc, mpi_comm_shim c){
                return context_shim(arb::make_context(alloc, c.comm));
            }),
             "alloc"_a, "comm"_a,
             "Construct a distributed context with arguments:\n"
             "  alloc:   The computational resources to be used for the simulation.\n"
             "  comm:    The MPI communicator.\n")
        .def(pybind11::init(
            [](int threads, pybind11::object gpu, pybind11::object mpi){
                opt_int gpu_id = py2optional<int>(gpu,
                        "gpu must be None, or a non-negative integer.", is_nonneg_int);
                arb::proc_allocation alloc(threads, gpu_id.value_or(-1));

                if (mpi.is_none()) {
                    return context_shim(arb::make_context(alloc));
                }
                opt_mpi_comm c = py2optional<mpi_comm_shim>(mpi,
                        "mpi must be None, or an MPI communicator.");
                auto comm = c.value_or(MPI_COMM_WORLD).comm;
                return context_shim(arb::make_context(alloc, comm));
            }),
             "threads"_a=1, "gpu"_a=pybind11::none(), "mpi"_a=pybind11::none(),
             "Construct a distributed context with arguments:\n"
             "  threads: The number of threads available locally for execution (defaults to 1).\n"
             "  gpu:     The index of the GPU to use (defaults to None for no GPU).\n"
             "  mpi:     The MPI communicator (defaults to None for no MPI).\n")
#else
        .def(pybind11::init(
            [](int threads, pybind11::object gpu){
                opt_int gpu_id = py2optional<int>(gpu,
                        "gpu must be None, or a non-negative integer.", is_nonneg_int);
                return context_shim(arb::make_context(arb::proc_allocation(threads, gpu_id.value_or(-1))));
            }),
             "threads"_a=1, "gpu"_a=pybind11::none(),
             "Construct a local context with arguments:\n"
             "  threads: The number of threads available locally for execution (defaults to 1).\n"
             "  gpu:     The index of the GPU to use (defaults to None for no GPU).\n")
#endif
        .def_property_readonly("has_mpi", [](const context_shim& ctx){return arb::has_mpi(ctx.context);},
            "Whether the context uses MPI for distributed communication.")
        .def_property_readonly("has_gpu", [](const context_shim& ctx){return arb::has_gpu(ctx.context);},
            "Whether the context has a GPU.")
        .def_property_readonly("threads", [](const context_shim& ctx){return arb::num_threads(ctx.context);},
            "The number of threads in the context's thread pool.")
        .def_property_readonly("ranks", [](const context_shim& ctx){return arb::num_ranks(ctx.context);},
            "The number of distributed domains (equivalent to the number of MPI ranks).")
        .def_property_readonly("rank", [](const context_shim& ctx){return arb::rank(ctx.context);},
            "The numeric id of the local domain (equivalent to MPI rank).")
        .def("__str__", [](const context_shim& c){return context_string(c.context);})
        .def("__repr__", [](const context_shim& c){return context_string(c.context);});
}

} // namespace pyarb
