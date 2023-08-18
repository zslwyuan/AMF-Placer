#include "PlacementTimingInfo.h"
#include "PlacementTimingOptimizer.h"
#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
   pybind11::class_<PlacementTimingOptimizer>(m, "PlacementTimingOptimizer")
       .def(py::init<PlacementInfo *, std::map<std::string, std::string> &>())
       .def("clusterLongPathInOneClockRegion", &PlacementTimingOptimizer::clusterLongPathInOneClockRegion)
       .def("conductStaticTimingAnalysis", &PlacementTimingOptimizer::conductStaticTimingAnalysis);

   pybind11::class_<PlacementTimingInfo>(m, "PlacementTimingInfo")
       .def(py::init<DesignInfo *, DeviceInfo *, std::map<std::string, std::string> &>())
       .def("setDSPInnerDelay", &PlacementTimingInfo::setDSPInnerDelay);
}
#endif