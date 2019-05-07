#include <iostream>
#include <sstream>
#include <string>

#include <arbor/context.hpp>
#include <arbor/version.hpp>

#include <pybind11/pybind11.h>

#include "context.hpp"
#include "exception.hpp"
#include "strings.hpp"

#ifdef ARB_MPI_ENABLED
#include "mpi.hpp"
#ifdef ARB_WITH_MPI4PY
#include <mpi4py/mpi4py.h>
#endif
#endif

namespace pyarb {

auto is_int_or_minone = [](auto&& t){ return ((t==int(t) and t>=0) or t==-1); };

#ifdef ARB_MPI_ENABLED
#ifdef ARB_WITH_MPI4PY
auto is_mpi_comm = [](auto&& t){
    import_mpi4py();
    return PyObject_TypeCheck(t.comm, &PyMPIComm_Type);
};
#else
auto is_mpi_comm = [](auto&& t){
    return int (t)==t;
};
#endif
#endif

void register_contexts(pybind11::module& m) {
    using namespace std::string_literals;
    using namespace pybind11::literals;
    using opt_int = arb::util::optional<int>;
    using opt_mpi_comm = arb::util::optional<mpi_comm_shim&>;

    pybind11::class_<arb::proc_allocation> proc_allocation(m, "proc_allocation");
    proc_allocation
        .def(pybind11::init<>())
        .def(pybind11::init(
            [](int threads, opt_int gpu){
                int gpu_id = pyarb::assert_predicate(gpu.value_or(-1), is_int_or_minone, "gpu must be None, or a non-negative integer.");
                return arb::proc_allocation(threads, gpu_id);
            }),
            "threads"_a=1, "gpu"_a=pybind11::none(),
             "Arguments:\n"
             "  threads: The number of threads available locally for execution, defaults to 1.\n"
             "  gpu:     The index of the GPU to use, defaults to None for no GPU.\n")
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
            [](){return context_shim(arb::make_context());}))
        .def(pybind11::init(
            [](const arb::proc_allocation& alloc){return context_shim(arb::make_context(alloc));}),
             "alloc"_a,
             "Argument:\n"
             "  alloc:   The computational resources to be used for the simulation.\n")
#ifdef ARB_MPI_ENABLED
        .def(pybind11::init(
            [](const arb::proc_allocation& alloc, mpi_comm_shim c){return context_shim(arb::make_context(alloc, c.comm));}),
             "alloc"_a, "c"_a,
             "Arguments:\n"
             "  alloc:   The computational resources to be used for the simulation.\n"
             "  c:       The MPI communicator.\n")
        .def(pybind11::init(
            [](int threads, opt_int gpu, opt_mpi_comm mpi){
//            [](int threads, opt_int gpu, pybind11::object mpi){
                arb::proc_allocation alloc(threads, pyarb::assert_predicate(gpu.value_or(-1), is_int_or_minone, "gpu must be None, or a non-negative integer."));
/*                if (mpi.is_none()) {
                    return context_shim(arb::make_context(alloc));
                }
                auto& c = pybind11::cast<mpi_comm_shim&>(mpi);
                return context_shim(arb::make_context(alloc, c.comm));
*/
#ifdef ARB_WITH_MPI4PY
// communicator is pointer
                auto& c = pyarb::assert_predicate(mpi.value_or(MPI_COMM_WORLD), is_mpi_comm, "mpi must be None, or an MPI communicator.");
#else
// communicator is integer
                auto& c = pyarb::assert_predicate(mpi.value_or(MPI_COMM_WORLD), is_mpi_comm, "mpi must be None, or an MPI communicator.");             
#endif
                if (!c.comm) {
                    return context_shim(arb::make_context(alloc));
                }
                else return context_shim(arb::make_context(alloc, c.comm));
            }),
             "threads"_a=1, "gpu"_a=pybind11::none(), "mpi"_a=pybind11::none(),
             "Arguments:\n"
             "  threads: The number of threads available locally for execution, defaults to 1.\n"
             "  gpu:     The index of the GPU to use, defaults to None for no GPU.\n"
             "  mpi:     The MPI communicator, defaults to None for no MPI.\n")
#else
        .def(pybind11::init(
            [](int threads, opt_int gpu){
                int gpu_id = pyarb::assert_predicate(gpu.value_or(-1), is_int_or_minone, "gpu must be None, or a non-negative integer.");
                return context_shim(arb::make_context(arb::proc_allocation(threads, gpu_id)));
            }),
             "threads"_a=1, "gpu"_a=pybind11::none(),
             "Arguments:\n"
             "  threads: The number of threads available locally for execution, defaults to 1.\n"
             "  gpu:     The index of the GPU to use, defaults to None for no GPU.\n")
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
