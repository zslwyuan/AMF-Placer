#include "GlobalPlacer.h"
#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    pybind11::class_<GlobalPlacer>(m, "GlobalPlacer")
       .def(py::init<PlacementInfo *, std::map<std::string, std::string> &, bool>())
       .def("clusterPlacement", &GlobalPlacer::clusterPlacement)
       .def("GlobalPlacement_fixedCLB", &GlobalPlacer::GlobalPlacement_fixedCLB)
       .def("GlobalPlacement_CLBElements", &GlobalPlacer::GlobalPlacement_CLBElements)
       .def("setPseudoNetWeight", &GlobalPlacer::setPseudoNetWeight)
       .def("setMacroLegalizationParameters", &GlobalPlacer::setMacroLegalizationParameters)
       .def("getPseudoNetWeight", &GlobalPlacer::getPseudoNetWeight)
       .def("getMacroPseudoNetEnhanceCnt", &GlobalPlacer::getMacroPseudoNetEnhanceCnt)
       .def("getMacroLegalizationWeight", &GlobalPlacer::getMacroLegalizationWeight)
       .def("setNeighborDisplacementUpperbound", &GlobalPlacer::setNeighborDisplacementUpperbound)
       .def("getWirelengthOptimizer", &GlobalPlacer::getWirelengthOptimizer, pybind11::return_value_policy::reference);
    pybind11::class_<WirelengthOptimizer>(m, "WirelengthOptimizer")
       .def(py::init<PlacementInfo *, std::map<std::string, std::string>, bool>());
  // FFSRCompatible
}
#endif