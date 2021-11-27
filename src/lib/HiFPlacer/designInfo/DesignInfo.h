/**
 * @file DesignInfo.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the classes of data for a standalone design netlist.
 * @version 0.1
 * @date 2021-06-03
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _DESIGNINFO
#define _DESIGNINFO

#include "DeviceInfo.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define CELLTYPESTRS                                                                                                   \
    "LUT1", "LUT2", "LUT3", "LUT4", "LUT5", "LUT6", "LUT6_2", "FDCE", "FDPE", "FDRE", "FDSE", "LDCE", "AND2B1L",       \
        "CARRY8", "DSP48E2", "MUXF7", "MUXF8", "SRL16E", "SRLC32E", "RAM32M16", "RAM64M", "RAM64X1D", "RAM32M",        \
        "RAM32X1D", "RAM32X1S", "RAM64X1S", "RAM64M8", "FIFO36E2", "FIFO18E2", "RAMB18E2", "RAMB36E2", "BUFGCE",       \
        "BUFG_GT", "BUFG_GT_SYNC", "BUFGCE_DIV", "BUFGCTRL", "GTHE3_CHANNEL", "GTHE3_COMMON", "IOBUF", "IBUF",         \
        "IBUFDS", "IOBUFDS", "IBUFDS_GTE3", "IBUF_ANALOG", "IOBUFE3", "MMCME3_ADV", "OBUF", "OBUFT", "PCIE_3_1",       \
        "BSCANE2", "SYSMONE1", "RXTX_BITSLICE", "BITSLICE_CONTROL", "TX_BITSLICE_TRI", "OSERDESE3", "RIU_OR",          \
        "PLLE3_ADV", "HPIO_VREF", "OBUFDS_DUAL_BUF"

/**
 * @brief Information related to FPGA designs, including design cells and their interconnections.
 *
 */
class DesignInfo
{
  public:
    /**
     * @brief types of the elements in a design
     *
     *  There are cells in a design, pins of which are interconnected via nets.
     */
    enum DesignElementType
    {
        ElementType_cell = 0,
        ElementType_pin,
        ElementType_net,
        ElementType_graph
    };

    /**
     * @brief design cell types
     *
     * This is a design concept. These cell types should be mapped to device BEL types (resour concept).
     *
     */
    enum DesignCellType // have to have the the same order with CELLTYPESTRS macro
    {
        CellType_LUT1 = 0,
        CellType_LUT2,
        CellType_LUT3,
        CellType_LUT4,
        CellType_LUT5,
        CellType_LUT6,
        CellType_LUT6_2, // these are LUTs

        CellType_FDCE,
        CellType_FDPE,
        CellType_FDRE,
        CellType_FDSE,
        CellType_LDCE,
        CellType_AND2B1L, // these are FFs

        CellType_CARRY8,
        CellType_DSP48E2,

        CellType_MUXF7,
        CellType_MUXF8,

        CellType_SRL16E,
        CellType_SRLC32E, // these are shifters

        CellType_RAM32M16,
        CellType_RAM64M,
        CellType_RAM64X1D,
        CellType_RAM32M,
        CellType_RAM32X1D,
        CellType_RAM32X1S,
        CellType_RAM64X1S,
        CellType_RAM64M8, // these are LUTRAMs

        CellType_FIFO36E2,
        CellType_FIFO18E2,
        CellType_RAMB18E2,
        CellType_RAMB36E2, // these are BRAMs

        CellType_BUFGCE,
        CellType_BUFG_GT,
        CellType_BUFG_GT_SYNC,
        CellType_BUFGCE_DIV,
        CellType_BUFGCTRL, // clock-related types

        CellType_GTHE3_CHANNEL,
        CellType_GTHE3_COMMON,
        CellType_IOBUF,
        CellType_IBUF,
        CellType_IBUFDS,
        CellType_IOBUFDS,
        CellType_IBUFDS_GTE3,
        CellType_IBUF_ANALOG,
        CellType_IOBUFE3,

        CellType_MMCME3_ADV,

        CellType_OBUF,
        CellType_OBUFT,
        CellType_PCIE_3_1,

        CellType_BSCANE2,
        CellType_SYSMONE1,
        CellType_RXTX_BITSLICE,
        CellType_BITSLICE_CONTROL,
        CellType_TX_BITSLICE_TRI,
        CellType_OSERDESE3,

        CellType_RIU_OR,
        CellType_PLLE3_ADV,
        CellType_HPIO_VREF,
        CellType_OBUFDS_DUAL_BUF
    };

    inline static bool FFSRCompatible(DesignCellType typeA, DesignCellType typeB)
    {
        // CellType_FDCE,
        // CellType_FDPE,
        // CellType_FDRE,
        // CellType_FDSE,
        // CellType_LDCE,
        // CellType_AND2B1L, // these are FFs
        int AClass = -1;
        if (typeA == CellType_FDCE || typeA == CellType_FDPE)
            AClass = 0;
        else if (typeA == CellType_FDRE || typeA == CellType_FDSE)
            AClass = 1;
        else if (typeA == CellType_LDCE)
            AClass = 2;
        else if (typeA == CellType_AND2B1L)
            AClass = 3;
        assert(AClass != -1);
        int BClass = -1;
        if (typeB == CellType_FDCE || typeB == CellType_FDPE)
            BClass = 0;
        else if (typeB == CellType_FDRE || typeB == CellType_FDSE)
            BClass = 1;
        else if (typeB == CellType_LDCE)
            BClass = 2;
        else if (typeB == CellType_AND2B1L)
            BClass = 3;
        assert(BClass != -1);
        return AClass == BClass;
    }

    inline static bool getFFSRType(DesignCellType typeA)
    {
        int AClass = -1;
        if (typeA == CellType_FDCE || typeA == CellType_FDPE)
            AClass = 0;
        else if (typeA == CellType_FDRE || typeA == CellType_FDSE)
            AClass = 1;
        else if (typeA == CellType_LDCE)
            AClass = 2;
        else if (typeA == CellType_AND2B1L)
            AClass = 3;
        assert(AClass != -1);
        return AClass;
    }

    std::vector<std::string> DesignCellTypeStr{CELLTYPESTRS};

    enum DesignPinType
    {
        PinType_LUTInput = 0,
        PinType_LUTOutput,
        PinType_CLK, // FF clk
        PinType_Q,
        PinType_D,
        PinType_E,  // FF enable
        PinType_SR, // FF reset/set
        PinType_Others
    };

    /**
     * @brief basic class of element in a design.
     *
     * includes the name, id and parent of an element.
     *
     */
    class DesignElement
    {
      public:
        DesignElement(const std::string &name, DesignElement *parentPtr, DesignElementType type, int id)
            : name(name), parentPtr(parentPtr), type(type), id(id)
        {
        }

        DesignElement(const std::string &name, DesignElementType type, int id)
            : name(name), parentPtr(nullptr), type(type), id(id)
        {
        }
        DesignElement(bool isVirtual, DesignElementType type, int id) : parentPtr(nullptr), type(type), id(id)
        {
            assert(isVirtual);
            name = std::to_string(id);
        }

        DesignElement(bool isVirtual, const std::string &_name, DesignElementType type, int id)
            : parentPtr(nullptr), type(type), id(id)
        {
            assert(isVirtual);
            name = _name + "(" + std::to_string(id) + ")";
        }

        virtual ~DesignElement()
        {
        }

        inline const std::string &getName() const
        {
            return name;
        }
        inline DesignElement *getParentPtr()
        {
            return parentPtr;
        }
        inline DesignElementType getElementType()
        {
            return type;
        }
        inline int getElementIdInType()
        {
            return id;
        }

      private:
        std::string name;
        DesignElement *parentPtr = nullptr;
        DesignElementType type;
        int id;
    };

    class DesignPin;
    class DesignNet;
    class DesignCell;
    class ControlSetInfo;

    /**
     * @brief A design pin on a design cell connected to a net
     *
     */
    class DesignPin : public DesignElement
    {
      public:
        /**
         * @brief Construct a new Design Pin object
         *
         * @param name the actual/unique name of design pin
         * @param refpinname the reference name of the pin on the cell, indicating which device pin shoud be used on
         * the device BEL
         * @param pinType some pins can be classified into specific type (clk, reset...)
         * @param inputOrNot
         * @param parentPtr the cell of the pin
         * @param id the id of the pin in the pin list
         */
        DesignPin(std::string &name, std::string &refpinname, DesignPinType pinType, bool inputOrNot,
                  DesignElement *parentPtr, int id)
            : DesignElement(name, parentPtr, ElementType_pin, id), pinType(pinType), refpinname(refpinname),
              inputOrNot(inputOrNot)
        {
        }

        ~DesignPin()
        {
        }

        /**
         * @brief get the pin type based on its reference name
         *
         * @param cell the cell of the pin
         * @param refpinname the reference name of the pin on the cell, indicating which device pin shoud be used on
         * the device BEL
         * @param isInput
         * @return DesignPinType
         */
        static DesignPinType checkPinType(DesignCell *cell, std::string &refpinname, bool isInput);

        /**
         * @brief Get the Pin Type of the pin
         *
         * @return DesignPinType
         */
        inline DesignPinType getPinType()
        {
            return pinType;
        }

        /**
         * @brief Get the name of the net which the pin connects to
         *
         * @return std::string&
         */
        inline std::string &getNetName()
        {
            return netName;
        }

        /**
         * @brief bind the pin to the net by the net name
         *
         * @param _netName
         */
        inline void connectToNetName(std::string &_netName)
        {
            netName = _netName;
        }

        inline int getAliasNetId()
        {
            return aliasNetId;
        }

        inline void setAliasNetId(int _aliasNetId)
        {
            aliasNetId = _aliasNetId;
        }

        /**
         * @brief bind the pin to the net's pointer for later processing
         *
         * @param _netPtr
         */
        inline void connectToNetVariable(DesignNet *_netPtr)
        {
            netPtr = _netPtr;
        }

        inline bool isOutputPort()
        {
            return !inputOrNot;
        }

        inline bool isInputPort()
        {
            return inputOrNot;
        }

        /**
         * @brief Set the driver Pin Name of the pin
         *
         * @param _driverPinName
         */
        inline void setDriverPinName(std::string &_driverPinName)
        {
            driverPinName = _driverPinName;
        }

        /**
         * @brief Set the Driver Pin object
         *
         * @param _driverPinPtr
         */
        inline void setDriverPin(DesignPin *_driverPinPtr)
        {
            driverPin = _driverPinPtr;
        }

        /**
         * @brief Get the driver pin of the pin (nullptr indicates that the pin connects to some global signal like
         * VGG/GND)
         *
         * @return DesignPin*
         */
        inline DesignPin *getDriverPin()
        {
            return driverPin;
        }

        /**
         * @brief let the parent cell know that one of its pin connects to a specific net
         *
         */
        void updateParentCellNetInfo();
        inline DesignNet *getNet()
        {
            return netPtr;
        };

        /**
         * @brief Get the Cell object of the pin
         *
         * @return DesignCell*
         */
        inline DesignCell *getCell()
        {
            DesignElement *tmpElement = getParentPtr();
            DesignCell *parentCell = dynamic_cast<DesignCell *>(tmpElement);
            assert(parentCell);
            return parentCell;
        }

        /**
         * @brief Get the reference pin name of the pin
         *
         * @return std::string&
         */
        std::string &getRefPinName()
        {
            return refpinname;
        }

        /**
         * @brief Set the attribute unconnected to be true, indicating the pin connect to no net
         *
         */
        inline void setUnconnected()
        {
            unconnected = true;
        }

        /**
         * @brief check if the attribute unconnected is true
         *
         * @return true indicating the pin connect to no net
         * @return false indicating the pin connect to a net, but note that the net could be nullptr(not design net but
         * a global signal)
         */
        inline bool isUnconnected()
        {
            return unconnected;
        }

        /**
         * @brief Set the Offset of the pin relative to the cell
         *
         * @param x
         * @param y
         */
        inline void setOffsetInCell(float x, float y)
        {
            offsetXInCell = x;
            offsetYInCell = y;
        }

        /**
         * @brief Get the offset X of the pin In the cell
         *
         * @return float
         */
        inline float getOffsetXInCell()
        {
            return offsetXInCell;
        }

        /**
         * @brief Get the offset Y of the pin In the cell
         *
         * @return float
         */
        inline float getOffsetYInCell()
        {
            return offsetYInCell;
        }

        inline bool isFixed()
        {
            return fixed;
        }

        inline void setFixed()
        {
            fixed = true;
        }

      private:
        /**
         * @brief pin type could be:
         *
         * PinType_LUTInput, PinType_LUTOutput, PinType_CLK, PinType_Q,PinType_D, PinType_E, PinType_SR
         *
         */
        DesignPinType pinType;
        std::string netName;
        std::string refpinname;
        DesignNet *netPtr = nullptr;
        bool inputOrNot;
        bool unconnected = false;
        bool fixed = false;
        std::string driverPinName;
        DesignPin *driverPin = nullptr;
        float offsetXInCell = 0.0;
        float offsetYInCell = 0.0;
        int aliasNetId = -1;
    };

    /**
     * @brief a design net (hyperedge) defined in the design, connecting to pins of cells
     *
     */
    class DesignNet : public DesignElement
    {
      public:
        /**
         * @brief Construct a new Design Net object
         *
         * the DesignNet will only initilize with name and id. Later it will bind to pins and cells.
         *
         * @param name the name of the net
         * @param id the Id of the net
         * @param virtualNet is it a virtual net not defined in the design?
         */
        DesignNet(std::string &name, int id, bool virtualNet = false)
            : DesignElement(name, ElementType_net, id), virtualNet(virtualNet)
        {
            if (name == "<const0>" || name == "<const1>")
            {
                isPowerNet = true;
            }
            overallClusterEnhanceRatio = 1.0;
            overallTimingEnhanceRatio = 1.0;
        }

        ~DesignNet()
        {
        }

        /**
         * @brief bind the net to a pin by name
         *
         * @param _pinName
         */
        void connectToPinName(const std::string &_pinName);
        /**
         * @brief bind the net to a pin's pointer
         *
         * be aware that the net could bind to multiple pins
         *
         * @param _pinPtr
         */
        void connectToPinVariable(DesignPin *_pinPtr);

        /**
         * @brief Get the vector reference of the driver pins of the net
         *
         * @return std::vector<DesignPin *>&
         */
        inline std::vector<DesignPin *> &getDriverPins()
        {
            return driverPinPtrs;
        }

        /**
         * @brief Get the vector reference of the pins driven by the net
         *
         * @return std::vector<DesignPin *>&
         */
        inline std::vector<DesignPin *> &getPinsBeDriven()
        {
            return BeDrivenPinPtrs;
        }

        /**
         * @brief Get the vector reference of the pins
         *
         * @return std::vector<DesignPin *>&
         */
        inline std::vector<DesignPin *> &getPins()
        {
            return pinPtrs;
        }

        inline bool isVirtual()
        {
            return virtualNet;
        }

        /**
         * @brief placer can customize some 2-pin interconnections to make their weights enhanced during wirelength
         * optimization
         *
         * @param pinIdInNetA pin A's id in the pin list of the net
         * @param pinIdInNetB pin B's id in the pin list of the net
         * @param ratio enhance the weight to what extent
         */
        inline void enhance(int pinIdInNetA, int pinIdInNetB, float ratio)
        {
            std::pair<int, int> tmpPair(pinIdInNetA, pinIdInNetB);
            if (pinIdPinIdInNet2EnhanceRatio.find(tmpPair) == pinIdPinIdInNet2EnhanceRatio.end())
            {
                pinIdPinIdInNet2EnhanceRatio[tmpPair] = ratio;
            }
            else
            {
                pinIdPinIdInNet2EnhanceRatio[tmpPair] *= ratio;
            }
        }

        /**
         * @brief Get the Pin Pair Enhance Ratio (placer can customize some 2-pin interconnections to make their weights
         * enhanced during wirelength optimization)
         *
         * @param pinIdInNetA pin A's id in the pin list of the net
         * @param pinIdInNetB pin B's id in the pin list of the net
         * @return float
         */
        inline float getPinPairEnhanceRatio(int pinIdInNetA, int pinIdInNetB)
        {
            std::pair<int, int> tmpPair(pinIdInNetA, pinIdInNetB);
            if (pinIdPinIdInNet2EnhanceRatio.find(tmpPair) == pinIdPinIdInNet2EnhanceRatio.end())
            {
                return 1.0;
            }
            else
            {
                return pinIdPinIdInNet2EnhanceRatio[tmpPair];
            }
        }

        /**
         * @brief Get the Overall Enhance Ratio (the entire net can be enhanced to a pre-defined extent.)
         *
         * @return float
         */
        inline float getOverallClusterEnhanceRatio()
        {
            return overallClusterEnhanceRatio;
        }

        /**
         * @brief Get the Overall Enhance Ratio (the entire net can be enhanced to a pre-defined extent.)
         *
         * @return float
         */
        inline float getOverallTimingEnhanceRatio()
        {
            return overallTimingEnhanceRatio;
        }

        /**
         * @brief Get the Overall Enhance Ratio (the entire net can be enhanced to a pre-defined extent.)
         *
         * @return float
         */
        inline float getOverallEnhanceRatio()
        {
            return overallClusterEnhanceRatio * overallTimingEnhanceRatio;
        }

        /**
         * @brief Set the Overall Net Enhancement (the entire net can be enhanced to a pre-defined extent.)
         *
         * @param r
         */
        inline void setOverallClusterNetEnhancement(float r)
        {
            overallClusterEnhanceRatio = r;
        }

        /**
         * @brief Set the Overall Net Enhancement (the entire net can be enhanced to a pre-defined extent.)
         *
         * @param r
         */
        inline void setOverallTimingNetEnhancement(float r)
        {
            overallTimingEnhanceRatio = r;
        }

        /**
         * @brief increase the Overall Net Enhancement (the entire net can be enhanced to a pre-defined extent.)
         *
         * @param r
         */
        inline void enhanceOverallClusterNetEnhancement(float r)
        {
            overallClusterEnhanceRatio *= r;
        }

        /**
         * @brief increase the Overall Net Enhancement (the entire net can be enhanced to a pre-defined extent.)
         *
         * @param r
         */
        inline void enhanceOverallTimingNetEnhancement(float r)
        {
            overallTimingEnhanceRatio *= r;
        }

        inline void resetEnhanceRatio()
        {
            pinIdPinIdInNet2EnhanceRatio.clear();
            overallClusterEnhanceRatio = 1;
            overallTimingEnhanceRatio = 1;
        }

        /**
         * @brief Set the attribute isGlobalClock to be true
         *
         * if the net is a global clock (which could CLK, SR, or CE signal)
         *
         */
        inline void setGlobalClock()
        {
            isGlobalClock = true;
        }

        /**
         * @brief check the attribute isGlobalClock
         *
         * @return true  if the net is a global clock (which could CLK, SR, or CE signal)
         * @return false  if the net is not a global clock
         */
        inline bool checkIsGlobalClock()
        {
            return isGlobalClock;
        }

        /**
         * @brief check whether the net is VCC or GND
         *
         * @return true if the net is VCC or GND
         * @return false if the net is NOT VCC or GND
         */
        inline bool checkIsPowerNet()
        {
            return isPowerNet;
        }

        inline bool checkContainFixedPins()
        {
            return containFixedPins;
        }

        inline void setContainFixedPins()
        {
            containFixedPins = true;
        }

      private:
        std::vector<std::string> pinNames;
        std::vector<DesignPin *> pinPtrs;
        std::vector<DesignPin *> driverPinPtrs;
        std::vector<DesignPin *> BeDrivenPinPtrs;
        bool containFixedPins = false;
        std::map<std::pair<int, int>, float> pinIdPinIdInNet2EnhanceRatio;
        float overallClusterEnhanceRatio = 1.0;
        float overallTimingEnhanceRatio = 1.0;
        bool virtualNet;
        bool isGlobalClock = false;
        bool isPowerNet = false;
    };

    /**
     * @brief a DesignCell in design netlist, DesignPin objects of which might connect to DesignNet objects
     *
     */
    class DesignCell : public DesignElement
    {
      public:
        /**
         * @brief Construct a new Design Cell object
         *
         * @param name the name of the cell
         * @param parentPtr the hierarchy parent of the design cell
         * @param cellType the design cell type (NOT resourse BEL type!)
         * @param id the id of cell in the cell list
         */
        DesignCell(const std::string &name, DesignElement *parentPtr, DesignCellType cellType, int id)
            : DesignElement(name, parentPtr, ElementType_cell, id), cellType(cellType)
        {
            pinPtrs.clear();
            inputPinPtrs.clear();
            outputPinPtrs.clear();
            pinNames.clear();
            netPtrs.clear();
            inputNetPtrs.clear();
            outputNetPtrs.clear();
            netNames.clear();
        }

        /**
         * @brief Construct a new Design Cell object
         *
         * @param name the name of the cell
         * @param cellType the design cell type (NOT resourse BEL type!)
         * @param id the id of cell in the cell list
         */
        DesignCell(const std::string &name, DesignCellType cellType, int id)
            : DesignElement(name, ElementType_cell, id), cellType(cellType)
        {
            oriCellType = cellType;
            pinPtrs.clear();
            inputPinPtrs.clear();
            outputPinPtrs.clear();
            pinNames.clear();
            netPtrs.clear();
            inputNetPtrs.clear();
            outputNetPtrs.clear();
            netNames.clear();
        }

        /**
         * @brief Construct a new Design Cell object without given name. The cell name will be its id string
         *
         * @param isVirtual indicate whether the cell is a virtual one not in the design netlist
         * @param cellType the design cell type (NOT resourse BEL type!)
         * @param id the id of cell in the cell list
         */
        DesignCell(bool isVirtual, DesignCellType cellType, int id)
            : DesignElement(isVirtual, ElementType_cell, id), cellType(cellType), isVirtual(isVirtual)
        {
            oriCellType = cellType;
            pinPtrs.clear();
            inputPinPtrs.clear();
            outputPinPtrs.clear();
            pinNames.clear();
            netPtrs.clear();
            inputNetPtrs.clear();
            outputNetPtrs.clear();
            netNames.clear();
            assert(isVirtual);
        }

        /**
         * @brief Construct a new Design Cell object
         *
         * @param isVirtual indicate whether the cell is a virtual one not in the design netlist
         * @param name the name of the cell
         * @param cellType the design cell type (NOT resourse BEL type!)
         * @param id the id of cell in the cell list
         */
        DesignCell(bool isVirtual, const std::string &name, DesignCellType cellType, int id)
            : DesignElement(isVirtual, name, ElementType_cell, id), cellType(cellType), isVirtual(isVirtual)
        {
            oriCellType = cellType;
            pinPtrs.clear();
            inputPinPtrs.clear();
            outputPinPtrs.clear();
            pinNames.clear();
            netPtrs.clear();
            inputNetPtrs.clear();
            outputNetPtrs.clear();
            netNames.clear();
            assert(isVirtual);
        }

        /**
         * @brief Destroy the Design Cell object and its binded DesignPin objects
         *
         */
        ~DesignCell()
        {
            for (auto pin : pinPtrs)
                delete pin;
        }

        inline DesignCellType getCellType()
        {
            return cellType;
        }

        /**
         * @brief bind a pin to the cell
         *
         * @param _pinPtr
         */
        void addPin(DesignPin *_pinPtr);

        inline std::vector<DesignPin *> &getPins()
        {
            return pinPtrs;
        }

        /**
         * @brief add a net and assign it into output/input net list
         *
         * @param _pinPtr
         * @param _netPtr
         */
        void addNetForPin(DesignPin *_pinPtr, DesignNet *_netPtr);

        inline std::vector<DesignNet *> &getOutputNets()
        {
            return outputNetPtrs;
        }
        inline std::vector<DesignNet *> &getInputNets()
        {
            return inputNetPtrs;
        }
        inline std::vector<DesignPin *> &getOutputPins()
        {
            return outputPinPtrs;
        }
        inline std::vector<DesignPin *> &getInputPins()
        {
            return inputPinPtrs;
        }

        inline bool isLUT6()
        {
            return cellType == CellType_LUT6 || cellType == CellType_LUT6_2;
        }
        inline bool isLUT()
        {
            return cellType == CellType_LUT6_2 || cellType == CellType_LUT6 || cellType == CellType_LUT5 ||
                   cellType == CellType_LUT4 || cellType == CellType_LUT3 || cellType == CellType_LUT2 ||
                   cellType == CellType_LUT1;
        }
        inline bool isFF()
        {
            return cellType == CellType_FDCE || cellType == CellType_FDPE || cellType == CellType_FDSE ||
                   cellType == CellType_LDCE || cellType == CellType_FDRE || cellType == CellType_AND2B1L;
        }
        inline bool isLUTRAM()
        {
            return (cellType == CellType_RAM32M16 || cellType == CellType_RAM32X1D || cellType == CellType_RAM64X1S ||
                    cellType == CellType_RAM64M || cellType == CellType_RAM64X1D || cellType == CellType_RAM32M ||
                    cellType == CellType_RAM32X1D || cellType == CellType_RAM32X1S || cellType == CellType_RAM64X1S ||
                    cellType == CellType_RAM64M8);
        }
        inline bool originallyIsLUTRAM()
        {
            return (oriCellType == CellType_RAM32M16 || oriCellType == CellType_RAM32X1D ||
                    oriCellType == CellType_RAM64X1S || oriCellType == CellType_RAM64M ||
                    oriCellType == CellType_RAM64X1D || oriCellType == CellType_RAM32M ||
                    oriCellType == CellType_RAM32X1D || oriCellType == CellType_RAM32X1S ||
                    oriCellType == CellType_RAM64X1S || oriCellType == CellType_RAM64M8);
        }
        inline bool isBRAM()
        {
            return cellType == CellType_RAMB18E2 || cellType == CellType_RAMB36E2 || cellType == CellType_FIFO18E2 ||
                   cellType == CellType_FIFO36E2;
        }
        inline bool isMux()
        {
            return cellType == CellType_MUXF7 || cellType == CellType_MUXF8;
        }
        inline bool isCarry()
        {
            return cellType == CellType_CARRY8;
        }
        inline bool isShift()
        {
            return cellType == CellType_SRL16E || cellType == CellType_SRLC32E;
        }
        inline bool isDSP()
        {
            return cellType == CellType_DSP48E2;
        }
        inline bool isIO()
        {
            return cellType == CellType_GTHE3_CHANNEL || cellType == CellType_GTHE3_COMMON ||
                   cellType == CellType_IOBUF || cellType == CellType_IBUF || cellType == CellType_IBUFDS ||
                   cellType == CellType_IOBUFDS || cellType == CellType_IBUFDS_GTE3 ||
                   cellType == CellType_IBUF_ANALOG || cellType == CellType_IOBUFE3 ||

                   cellType == CellType_MMCME3_ADV ||

                   cellType == CellType_OBUF || cellType == CellType_OBUFT || cellType == CellType_PCIE_3_1 ||

                   cellType == CellType_BSCANE2 || cellType == CellType_SYSMONE1 ||
                   cellType == CellType_RXTX_BITSLICE || cellType == CellType_BITSLICE_CONTROL ||
                   cellType == CellType_TX_BITSLICE_TRI || cellType == CellType_OSERDESE3 ||

                   cellType == CellType_RIU_OR || cellType == CellType_PLLE3_ADV || cellType == CellType_HPIO_VREF ||
                   cellType == CellType_OBUFDS_DUAL_BUF;
        }

        inline bool isClockBuffer()
        {
            return cellType == CellType_BUFGCE || cellType == CellType_BUFG_GT || cellType == CellType_BUFG_GT_SYNC ||
                   cellType == CellType_BUFGCE_DIV || cellType == CellType_BUFGCTRL;
        }

        inline bool isShifter()
        {
            return cellType == CellType_SRL16E || cellType == CellType_SRLC32E;
        }

        /**
         * @brief check whether the cell is an endpoint in timing graph
         *
         * @return true
         * @return false
         */
        inline bool isTimingEndPoint()
        {
            return (isFF() || isLUTRAM() || originallyIsLUTRAM() || isBRAM() || isDSP() || isIO() || isClockBuffer() ||
                    isShifter());
        }

        /**
         * @brief check whether the cell is related to logic computation
         *
         * @return true
         * @return false
         */
        inline bool isLogicRelated()
        {
            return (isLUT() || isFF() || isLUTRAM() || originallyIsLUTRAM() || isBRAM() || isDSP() || isShifter());
        }

        /**
         * @brief Get the Cell Id in the cell list
         *
         * @return int
         */
        inline int getCellId()
        {
            return getElementIdInType();
        }

        inline bool isVirtualCell()
        {
            return isVirtual;
        }

        /**
         * @brief Get the Control Set Info object of this cell
         *
         * A control set is a combination of CLK, CE and SR signal. It could be nullptr (not related to control set)
         *
         * @return ControlSetInfo*
         */
        inline ControlSetInfo *getControlSetInfo()
        {
            return controlSetInfo;
        }

        /**
         * @brief Set the Control Set Info object  of this cell
         *
         * A control set is a combination of CLK, CE and SR signal. It could be nullptr (not related to control set)
         *
         * @param _controlSetInfo
         */
        inline void setControlSetInfo(ControlSetInfo *_controlSetInfo)
        {
            controlSetInfo = _controlSetInfo;
        }

        /**
         * @brief Set the Virtual Type object which might override the actual type in later processing
         *
         * for example, for some LUT1-5, we might set its virtual type to be LUT6 to ensure that it will be paired with
         * other LUTs.
         *
         * @param NewCellType
         */
        inline void setVirtualType(DesignCellType NewCellType)
        {
            cellType = NewCellType;
        }

        /**
         * @brief Get the Original Cell Type object defined in the design netlist
         *
         * @return DesignCellType
         */
        inline DesignCellType getOriCellType()
        {
            return oriCellType;
        }

        /**
         * @brief add a clock net which is connected to this cell for later legalization
         *
         * @param aClockNet
         */
        inline void addClockNet(DesignNet *aClockNet)
        {
            clockNetPtrs.insert(aClockNet);
        }

        /**
         * @brief Get the clock nets connected to this cell for later legalization
         *
         * @return std::vector<DesignNet *>&
         */
        inline std::set<DesignNet *> &getClockNets()
        {
            return clockNetPtrs;
        }

      private:
        std::vector<DesignPin *> pinPtrs;
        std::vector<DesignPin *> inputPinPtrs;
        std::vector<DesignPin *> outputPinPtrs;
        std::vector<std::string> pinNames;
        std::vector<DesignNet *> netPtrs;
        std::vector<DesignNet *> inputNetPtrs;
        std::vector<DesignNet *> outputNetPtrs;
        std::set<DesignNet *> clockNetPtrs;
        std::vector<std::string> netNames;
        DesignCellType cellType;
        DesignCellType oriCellType;
        bool isVirtual = false;
        ControlSetInfo *controlSetInfo = nullptr;
    };

    /**
     * @brief A control set is a combination of CLK, CE and SR signal. It could be nullptr (not related to control set)
     *
     */
    class ControlSetInfo
    {
      public:
        /**
         * @brief Construct a new Control Set Info object with signals of a given FF and a given ID
         *
         * @param curFF FF to extract control set signal
         * @param id assign an Id for this combination
         */
        ControlSetInfo(DesignInfo::DesignCell *curFF, int id) : id(id)
        {
            FFs.clear();
            setControlSetInfoAs(curFF);
        };

        ~ControlSetInfo(){};

        /**
         * @brief check if a FF is compatible with the control set
         *
         * check all the control signals of the FF are identical to those of control set respectively. Only those FFs
         * compatible to the same control set can be packed in one Half CLB block in CLB site.
         *
         * @param curFF a given FF
         * @return true if a FF is compatible with the control set
         * @return false if a FF is NOT compatible with the control set
         */
        inline bool compatibleWith(DesignInfo::DesignCell *curFF)
        {
            for (auto inputPin : curFF->getInputPins())
            {
                if (inputPin->getPinType() == DesignInfo::PinType_CLK)
                {
                    if (inputPin->isUnconnected() && CLK)
                        return false;
                    if (inputPin->getNet() != CLK)
                        return false;
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_E)
                {
                    if (inputPin->isUnconnected() && CE)
                        return false;
                    if (inputPin->getNet() != CE)
                        return false;
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_SR)
                {
                    if (inputPin->isUnconnected() && SR)
                        return false;
                    if (inputPin->getNet() != SR)
                        return false;
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_D)
                {
                    // bypass
                }
                else
                {
                    assert(false && "undefined FF input pin type.");
                }
            }
            if (FFSRCompatible(curFF->getOriCellType(), FFType))
                return false;
            return true;
        }

        /**
         * @brief Set the control set information accoridng to a given FF
         *
         * @param curFF target FF to extract control set signal
         */
        inline void setControlSetInfoAs(DesignInfo::DesignCell *curFF)
        {
            for (auto inputPin : curFF->getInputPins())
            {
                if (inputPin->getPinType() == DesignInfo::PinType_CLK)
                {
                    if (inputPin->isUnconnected())
                        CLK = nullptr;
                    CLK = inputPin->getNet();
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_E)
                {
                    if (inputPin->isUnconnected())
                        CE = nullptr;
                    CE = inputPin->getNet();
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_SR)
                {
                    if (inputPin->isUnconnected())
                        SR = nullptr;
                    SR = inputPin->getNet();
                }
                else if (inputPin->getPinType() == DesignInfo::PinType_D)
                {
                    // bypass
                }
                else
                {
                    assert(false && "undefined FF input pin type.");
                }
            }
            FFType = curFF->getOriCellType();
        }

        inline DesignInfo::DesignNet *getCLK() const
        {
            return CLK;
        }
        inline DesignInfo::DesignNet *getSR() const
        {
            return SR;
        }
        inline DesignInfo::DesignNet *getCE() const
        {
            return CE;
        }

        inline DesignInfo::DesignCellType getFFType() const
        {
            return FFType;
        }
        /**
         * @brief Get the Id of the control set (each control set will have a unique Id)
         *
         * @return const int
         */
        inline const int getId() const
        {
            return id;
        }

        /**
         * @brief add a FF into the set of FFs which are compatible to this control set
         *
         * @param curFF
         */
        inline void addFF(DesignInfo::DesignCell *curFF)
        {
            FFs.push_back(curFF);
        }

        /**
         * @brief get the set of FFs which are compatible to this control set
         *
         * @return std::vector<DesignInfo::DesignCell *>&
         */
        inline std::vector<DesignInfo::DesignCell *> &getFFs()
        {
            return FFs;
        }

        inline void display()
        {
            if (CLK)
                std::cout << "CLK: " << CLK->getName() << "\n";
            if (SR)
                std::cout << "SR: " << SR->getName() << "\n";
            if (CE)
                std::cout << "CE: " << CE->getName() << "\n";
            std::vector<std::string> DesignCellTypeStr{CELLTYPESTRS};
            std::cout << "CellType: " << DesignCellTypeStr[(int)FFType] << "\n";
        }

      private:
        DesignInfo::DesignNet *CLK = nullptr;
        DesignInfo::DesignNet *SR = nullptr;
        DesignInfo::DesignNet *CE = nullptr;
        DesignInfo::DesignCellType FFType;
        std::vector<DesignInfo::DesignCell *> FFs;
        int id = -1;
    };

    inline static bool isLUT(DesignCellType cellType)
    {
        return cellType == CellType_LUT6_2 || cellType == CellType_LUT6 || cellType == CellType_LUT5 ||
               cellType == CellType_LUT4 || cellType == CellType_LUT3 || cellType == CellType_LUT2 ||
               cellType == CellType_LUT1;
    }

    inline static bool isCarry(DesignCellType cellType)
    {
        return cellType == CellType_CARRY8;
    }

    inline static bool isDSP(DesignCellType cellType)
    {
        return cellType == CellType_DSP48E2;
    }

    inline static bool isBRAM(DesignCellType cellType)
    {
        return cellType == CellType_RAMB36E2 || cellType == CellType_RAMB18E2 || cellType == CellType_FIFO18E2 ||
               cellType == CellType_FIFO36E2;
    }

    inline static bool isMux(DesignCellType cellType)
    {
        return cellType == CellType_MUXF7 || cellType == CellType_MUXF8;
    }

    inline static bool isLUTRAM(DesignCellType cellType)
    {
        return (cellType == CellType_RAM32M16 || cellType == CellType_RAM32X1D || cellType == CellType_RAM64X1S ||
                cellType == CellType_RAM64M || cellType == CellType_RAM64X1D || cellType == CellType_RAM32M ||
                cellType == CellType_RAM32X1D || cellType == CellType_RAM32X1S || cellType == CellType_RAM64X1S ||
                cellType == CellType_RAM64M8);
    }

    inline static bool isFF(DesignCellType cellType)
    {
        return cellType == CellType_FDCE || cellType == CellType_FDPE || cellType == CellType_FDSE ||
               cellType == CellType_LDCE || cellType == CellType_FDRE || cellType == CellType_AND2B1L;
    }

    inline static bool isIO(DesignCellType cellType)
    {
        return cellType == CellType_GTHE3_CHANNEL || cellType == CellType_GTHE3_COMMON || cellType == CellType_IOBUF ||
               cellType == CellType_IBUF || cellType == CellType_IBUFDS || cellType == CellType_IOBUFDS ||
               cellType == CellType_IBUFDS_GTE3 || cellType == CellType_IBUF_ANALOG || cellType == CellType_IOBUFE3 ||

               cellType == CellType_MMCME3_ADV ||

               cellType == CellType_OBUF || cellType == CellType_OBUFT || cellType == CellType_PCIE_3_1 ||

               cellType == CellType_BSCANE2 || cellType == CellType_SYSMONE1 || cellType == CellType_RXTX_BITSLICE ||
               cellType == CellType_BITSLICE_CONTROL || cellType == CellType_TX_BITSLICE_TRI ||
               cellType == CellType_OSERDESE3 ||

               cellType == CellType_RIU_OR || cellType == CellType_PLLE3_ADV || cellType == CellType_HPIO_VREF ||
               cellType == CellType_OBUFDS_DUAL_BUF;
    }

    inline bool isClockBuffer(DesignCellType cellType)
    {
        return cellType == CellType_BUFGCE || cellType == CellType_BUFG_GT || cellType == CellType_BUFG_GT_SYNC ||
               cellType == CellType_BUFGCE_DIV || cellType == CellType_BUFGCTRL;
    }
    inline bool isShifter(DesignCellType cellType)
    {
        return cellType == CellType_SRL16E || cellType == CellType_SRLC32E;
    }

    /**
     * @brief check whether the cell type is an endpoint in timing graph
     *
     * @param cellType a given cell type
     * @return true
     * @return false
     */
    inline bool isTimingEndPoint(DesignCellType cellType)
    {
        return (isFF(cellType) || isLUTRAM(cellType) || isBRAM(cellType) || isDSP(cellType) || isIO(cellType) ||
                isClockBuffer(cellType) || isShifter(cellType));
    }

    /**
     * @brief check whether the cell is related to logic computation
     *
     * @param cellType a given cell type
     * @return true
     * @return false
     */
    inline bool isLogicRelated(DesignCellType cellType)
    {
        return (isLUT(cellType) || isFF(cellType) || isLUTRAM(cellType) || isBRAM(cellType) || isDSP(cellType) ||
                isShifter(cellType));
    }

    // clang-format off
    /**
     * @brief Construct a new Design Info object based on user-defined settings and device information
     *
     * @param JSONCfg the file of user-defined settings
     * @param deviceInfo device information

     * The design information file will contain information like the text shown below
        ~~~~{.tcl}
        curCell=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1
            type=> LUT4
            pin=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/O
            dir=> OUT
            net=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/next_fwft_state__0[0]
            drivepin=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/O
            pin=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/I0
            dir=> IN
            net=> design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/rd_en
            drivepin=> design_1_i/face_detect_0/inst/regslice_both_input_r_U/ibuf_inst/input_r_TREADY_INST_0/O
        ~~~~
     *
     *
     */
    // clang-format on
    DesignInfo(std::map<std::string, std::string> &JSONCfg, DeviceInfo *deviceInfo);

    ~DesignInfo()
    {
        for (auto net : netlist)
            delete net;
        for (auto cell : cells)
            delete cell;
        for (auto CS : controlSets)
            delete CS;
    }

    /**
     * @brief bind a pin to an existing net. If the net does not exist, new one.
     *
     * @param curPin target pin
     */
    void addPinToNet(DesignPin *curPin);

    /**
     * @brief translate a string into a DesignCellType for a cell
     *
     * @param cellName target cell
     * @param typeName type name string
     * @return DesignCellType
     */
    DesignCellType fromStringToCellType(std::string &cellName, std::string &typeName);

    /**
     * @brief add a cell into the design information
     *
     * @param curCell target cell
     * @return DesignCell* if there is duplicated object, delete the new cell and return the existing object
     */
    DesignCell *addCell(DesignCell *curCell);

    /**
     * @brief extract the ids of CLK, SR, and CE for a given FF
     *
     * @param curFF a given FF
     * @param CLKId output CLK Id
     * @param SRId  output SR Id
     * @param CEId  output CE Id
     */
    inline void getCLKSRCENetId(DesignInfo::DesignCell *curFF, int &CLKId, int &SRId, int &CEId)
    {
        assert(curFF->isFF());
        CLKId = -1;
        SRId = -1;
        CEId = -1;
        for (auto inputPin : curFF->getInputPins())
        {
            if (inputPin->getPinType() == DesignInfo::PinType_CLK)
            {
                if (!inputPin->isUnconnected())
                    CLKId = inputPin->getNet()->getElementIdInType();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_E)
            {
                if (!inputPin->isUnconnected())
                    CEId = inputPin->getNet()->getElementIdInType();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_SR)
            {
                if (!inputPin->isUnconnected())
                    SRId = inputPin->getNet()->getElementIdInType();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_D)
            {
                // bypass
            }
            else
            {
                assert(false && "undefined FF input pin type.");
            }
        }
    }

    /**
     * @brief extract the pointers of CLK, SR, and CE for a given FF
     *
     * @param curFF a given FF
     * @param CLK output CLK pointer
     * @param SR  output SR pointer
     * @param CE  output CE pointer
     */
    inline void getCLKSRCENet(DesignInfo::DesignCell *curFF, DesignNet **CLK, DesignNet **SR, DesignNet **CE)
    {
        assert(curFF->isFF());
        *CLK = nullptr;
        *SR = nullptr;
        *CE = nullptr;
        for (auto inputPin : curFF->getInputPins())
        {
            if (inputPin->getPinType() == DesignInfo::PinType_CLK)
            {
                if (!inputPin->isUnconnected())
                    *CLK = inputPin->getNet();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_E)
            {
                if (!inputPin->isUnconnected())
                    *CE = inputPin->getNet();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_SR)
            {
                if (!inputPin->isUnconnected())
                    *SR = inputPin->getNet();
            }
            else if (inputPin->getPinType() == DesignInfo::PinType_D)
            {
                // bypass
            }
            else
            {
                assert(false && "undefined FF input pin type.");
            }
        }
    }

    /**
     * @brief load the global clock signals from a design information file
     *
     * @param clockFileName
     */
    void loadClocks(std::string clockFileName);

    /**
     * @brief go through the FF cells to extract control sets for later processing
     *
     */
    void updateFFControlSets();

    /**
     * @brief intend to enhance the nets between FFs in a control set to make later packing easier
     *
     */
    void enhanceFFControlSetNets();

    /**
     * @brief Get the control sets in the design
     *
     * @return std::vector<ControlSetInfo *>&
     */
    inline std::vector<ControlSetInfo *> &getControlSets()
    {
        return controlSets;
    }

    /**
     * @brief get the id of the control set of a given FF
     *
     * @param curFF target FF
     * @return int
     */
    inline int getFFControlSetId(DesignCell *curFF)
    {
        assert(curFF->isFF());
        assert(FFId2ControlSetId[curFF->getCellId()] >= 0);
        return FFId2ControlSetId[curFF->getCellId()];
    }

    /**
     * @brief for user-defined-cluster-based optimization, load the nets in a user-defined cluster for later
     * processing
     *
     */
    void loadUserDefinedClusterNets();

    inline int getNumCells()
    {
        return cells.size();
    };
    inline int getNumNets()
    {
        return netlist.size();
    };

    inline std::vector<DesignCell *> &getCells()
    {
        return cells;
    };
    inline std::vector<DesignNet *> &getNets()
    {
        return netlist;
    };
    inline std::vector<DesignPin *> &getPins()
    {
        return pins;
    };

    /**
     * @brief disable enhancement of all the nets in the design (reset extra weight to be 1)
     *
     */
    void resetNetEnhanceRatio()
    {
        for (auto net : netlist)
            net->resetEnhanceRatio();
    }

    void printStat(bool verbose = false);

    inline DesignCell *getCell(std::string &tmpName)
    {
        return name2Cell[tmpName];
    }

    /**
     * @brief Get the predefined clusters which are defined in design configuration files
     *
     * @return std::vector<std::vector<DesignCell *>>&
     */
    inline std::vector<std::vector<DesignCell *>> &getPredefinedClusters()
    {
        return predefinedClusters;
    }

    inline std::map<DesignCellType, std::vector<DesignCell *>> &getType2Cells()
    {
        return type2Cells;
    }

    /**
     * @brief reset the LUTFFDeterminedOccupation object
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     *
     */
    void resetLUTFFDeterminedOccupation()
    {
        LUTFFDeterminedOccupation.clear();
        LUTFFDeterminedOccupation.resize(cells.size(), -1);
    }

    /**
     * @briefget the Determined Occupation of a specific cell
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     * @param cellId target cell
     * @return int
     */
    inline int getDeterminedOccupation(int cellId)
    {
        return LUTFFDeterminedOccupation[cellId];
    }

    /**
     * @brief Set the Determined Occupation of a specific cell
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     *
     * @param cellId target cell
     * @param occupation resource demand of the cell after packing
     */
    inline void setDeterminedOccupation(int cellId, int occupation)
    {
        LUTFFDeterminedOccupation[cellId] = occupation;
    }

    /**
     * @brief check if a net is a global clock indicated by input design files
     *
     * @param tmpNet target net
     * @return true if a net is a global clock indicated by input design files
     * @return false if a net is NOT a global clock indicated by input design files
     */
    inline bool isDesignClock(DesignNet *tmpNet)
    {
        return clockSet.find(tmpNet) != clockSet.end();
    }

    /**
     * @brief Get the all the clock nets in the design
     *
     * @return std::vector<DesignNet *>&
     */
    inline std::vector<DesignNet *> &getClocksInDesign()
    {
        return clocks;
    }

    /**
     * @brief Get the cells driven by a given clock net
     *
     * @param clock a clock net
     * @return std::set<DesignCell *>&
     */
    inline std::set<DesignCell *> &getCellsUnderClock(DesignNet *clock)
    {
        assert(clock2Cells.find(clock) != clock2Cells.end());
        return clock2Cells[clock];
    }

  private:
    std::vector<DesignNet *> netlist;
    std::vector<DesignCell *> cells;
    std::vector<DesignPin *> pins;
    std::map<std::string, DesignNet *> name2Net;
    std::map<std::string, int> aliasNet2AliasNetId;
    std::map<std::string, DesignCell *> name2Cell;

    /**
     * @brief the predefined clusters which are defined in design configuration files
     *
     */
    std::vector<std::vector<DesignCell *>> predefinedClusters;
    std::map<DesignCellType, std::vector<DesignCell *>> type2Cells;

    /**
     * @brief connected pin pairs by nets with a small number of pins
     *
     */
    std::set<std::pair<DesignPin *, DesignPin *>> connectedPinsWithSmallNet;

    /**
     * @brief LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     *
     */
    std::vector<int> LUTFFDeterminedOccupation;

    /**
     * @brief the mapping from the tuple of CLK/SR/CE ids and FF type to a unique defined control set id
     *
     */
    std::map<std::tuple<int, int, int, int>, int> CLKSRCEFFType2ControlSetInfoId;

    /**
     * @brief the mapping from FF IDs to a unique defined control set id
     *
     */
    std::vector<int> FFId2ControlSetId;

    std::vector<ControlSetInfo *> controlSets;
    std::vector<DesignNet *> clocks;
    std::set<DesignNet *> clockSet;

    /**
     * @brief the mapping from clocks to their corresponding cells driven by the clock net
     *
     */
    std::map<DesignNet *, std::set<DesignCell *>> clock2Cells;

    std::map<std::string, std::string> &JSONCfg;
    std::string designArchievedTextFileName;
};

std::ostream &operator<<(std::ostream &os, DesignInfo::DesignCell *cell);
std::ostream &operator<<(std::ostream &os, DesignInfo::DesignPin *pin);
#endif