#include "DeviceInfo.h"
#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
   pybind11::class_<DeviceInfo>(m, "DeviceInfo")
       .def(py::init<std::map<std::string, std::string>&, std::string>())
        .def("printStat", &DeviceInfo::printStat, pybind11::arg("verbose") = false);
  // FFSRCompatible
}
#endif