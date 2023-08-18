#include "InitialPacker.h"
#include "IncrementalBELPacker.h"
#include "ParallelCLBPacker.h"
//#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
   pybind11::class_<InitialPacker>(m, "InitialPacker")
       .def(py::init<DesignInfo *, DeviceInfo *, PlacementInfo*, std::map<std::string, std::string> &>())
       .def("pack", &InitialPacker::pack);
   pybind11::class_<IncrementalBELPacker>(m, "IncrementalBELPacker")
       .def(py::init<DesignInfo *, DeviceInfo *, PlacementInfo*, std::map<std::string, std::string> &>())
       .def("LUTFFPairing", &IncrementalBELPacker::LUTFFPairing)
       .def("FFPairing", &IncrementalBELPacker::FFPairing);
   pybind11::class_<ParallelCLBPacker>(m, "ParallelCLBPacker")
       .def(py::init<DesignInfo *, DeviceInfo *, PlacementInfo*, std::map<std::string, std::string> &, 
                    int, int, float, float, float, int, float, std::string,
                    PlacementTimingOptimizer *, WirelengthOptimizer *>())
       .def("packCLBs", &ParallelCLBPacker::packCLBs)
       .def("setPULocationToPackedSite", &ParallelCLBPacker::setPULocationToPackedSite)
       .def("updatePackedMacro", &ParallelCLBPacker::updatePackedMacro);
  // FFSRCompatible
}
//#endif