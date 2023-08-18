#include "DesignInfo.h"

#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    pybind11::class_<DesignInfo>(m, "DesignInfo")
        .def(py::init<std::map<std::string, std::string>&, DeviceInfo*>())
        .def("fromStringToCellType", &DesignInfo::fromStringToCellType);
   // FFSRCompatible
    //m.def("print", &print, "compute pin RUDY map");
}
#endif