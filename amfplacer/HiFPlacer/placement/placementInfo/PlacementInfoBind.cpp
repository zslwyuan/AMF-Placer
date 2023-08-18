#include "PlacementInfo.h"
#if CMAKE_PYBIND11_TYPE
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
   pybind11::class_<PlacementInfo>(m, "PlacementInfo")
       .def(py::init<DesignInfo *, DeviceInfo *, std::map<std::string, std::string> &>())
       .def("setPaintDataBase", &PlacementInfo::setPaintDataBase)
       .def("resetLUTFFDeterminedOccupation", &PlacementInfo::resetLUTFFDeterminedOccupation)
       .def("printStat", &PlacementInfo::printStat)
       .def("createGridBins", &PlacementInfo::createGridBins)
       .def("verifyDeviceForDesign", &PlacementInfo::verifyDeviceForDesign)
       .def("buildSimpleTimingGraph", &PlacementInfo::buildSimpleTimingGraph)
       .def("getLongPathThresholdLevel", &PlacementInfo::getLongPathThresholdLevel)
       .def("getTimingInfo", &PlacementInfo::getTimingInfo)
       .def("adjustLUTFFUtilization", &PlacementInfo::adjustLUTFFUtilization)
       .def("getPU2ClockRegionCenters", &PlacementInfo::getPU2ClockRegionCenters)
       .def("updateB2BAndGetTotalHPWL", &PlacementInfo::updateB2BAndGetTotalHPWL)
       .def("checkClockUtilization", &PlacementInfo::checkClockUtilization)
       .def("resetLUTFFDeterminedOccupation", &PlacementInfo::resetLUTFFDeterminedOccupation)
       .def("dumpOverflowClockUtilization", &PlacementInfo::dumpOverflowClockUtilization)
       .def("updateElementBinGrid", &PlacementInfo::updateElementBinGrid)
       .def("dumpCongestion", &PlacementInfo::dumpCongestion)
       .def("getMediumPathThresholdLevel", &PlacementInfo::getMediumPathThresholdLevel)
       .def("setDSPInnerDelay", &PlacementInfo::setDSPInnerDelay)
       .def("clearSomeAttributesCannotRecord", &PlacementInfo::clearSomeAttributesCannotRecord)
       .def("clearPU2ClockRegionCenters", &PlacementInfo::clearPU2ClockRegionCenters)
       .def("dumpPlacementUnitInformation", &PlacementInfo::dumpPlacementUnitInformation);

  // FFSRCompatible
}
#endif