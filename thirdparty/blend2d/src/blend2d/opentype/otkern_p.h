// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_OPENTYPE_OTKERN_P_H
#define BLEND2D_OPENTYPE_OTKERN_P_H

#include "../array_p.h"
#include "../support_p.h"
#include "../opentype/otcore_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal_opentype
//! \{

namespace BLOpenType {

// ============================================================================
// [BLOpenType::Table]
// ============================================================================

//! OpenType 'kern' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/kern
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6kern.html
struct KernTable {
  enum : uint32_t { kMinSize = 4 };

  struct WinTableHeader {
    UInt16 version;
    UInt16 tableCount;
  };

  struct MacTableHeader {
    F16x16 version;
    UInt32 tableCount;
  };

  struct WinGroupHeader {
    enum Coverage : uint8_t {
      kCoverageHorizontal   = 0x01u,
      kCoverageMinimum      = 0x02u,
      kCoverageCrossStream  = 0x04u,
      kCoverageOverride     = 0x08u,
      kCoverageReservedBits = 0xF0u
    };

    UInt16 version;
    UInt16 length;
    UInt8 format;
    UInt8 coverage;
  };

  struct MacGroupHeader {
    enum Coverage : uint8_t {
      kCoverageVertical     = 0x80u,
      kCoverageCrossStream  = 0x40u,
      kCoverageVariation    = 0x20u,
      kCoverageReservedBits = 0x1Fu
    };

    UInt32 length;
    UInt8 coverage;
    UInt8 format;
    UInt16 tupleIndex;
  };

  struct Pair {
    union {
      UInt32 combined;
      struct {
        UInt16 left;
        UInt16 right;
      };
    };
    Int16 value;
  };

  struct Format0 {
    UInt16 pairCount;
    UInt16 searchRange;
    UInt16 entrySelector;
    UInt16 rangeShift;
    /*
    Pair pairArray[pairCount];
    */

    BL_INLINE const Pair* pairArray() const noexcept { return blOffsetPtr<const Pair>(this, 8); }
  };

  struct Format1 {
    enum ValueBits : uint16_t {
      kValueOffsetMask      = 0x3FFFu,
      kValueNoAdvance       = 0x4000u,
      kValuePush            = 0x8000u
    };

    struct StateHeader {
      UInt16 stateSize;
      Offset16 classTable;
      Offset16 stateArray;
      Offset16 entryTable;
    };

    StateHeader stateHeader;
    Offset16 valueTable;
  };

  struct Format2 {
    struct ClassTable {
      UInt16 firstGlyph;
      UInt16 glyphCount;
      /*
      Offset16 offsetArray[glyphCount];
      */

      BL_INLINE const Offset16* offsetArray() const noexcept { return blOffsetPtr<const Offset16>(this, 4); }
    };

    UInt16 rowWidth;
    Offset16 leftClassTable;
    Offset16 rightClassTable;
    Offset16 kerningArray;
  };

  struct Format3 {
    UInt16 glyphCount;
    UInt8 kernValueCount;
    UInt8 leftClassCount;
    UInt8 rightClassCount;
    UInt8 flags;
    /*
    FWord kernValue[kernValueCount];
    UInt8 leftClass[glyphCount];
    UInt8 rightClass[glyphCount];
    UInt8 kernIndex[leftClassCount * rightClassCount];
    */
  };

  WinTableHeader header;
};

// ============================================================================
// [BLOpenType::KernGroup]
// ============================================================================

//! Kerning group.
//!
//! Helper data that we create for each kerning group (sub-table).
struct KernGroup {
  // Using the same bits as `KernTable::WinGroupHeader::Coverage` except for Horizontal.
  enum Flags : uint32_t {
    kFlagSynthesized = 0x01u,
    kFlagMinimum     = 0x02u,
    kFlagCrossStream = 0x04u,
    kFlagOverride    = 0x08u
  };

#if BL_TARGET_ARCH_BITS < 64
  uint32_t format : 2;
  uint32_t flags : 4;
  uint32_t dataSize : 26;
#else
  uint32_t format : 2;
  uint32_t flags : 30;
  uint32_t dataSize;
#endif

  union {
    uintptr_t dataOffset;
    void* dataPtr;
  };

  BL_INLINE bool hasFlag(uint32_t flag) const noexcept { return (flags & flag) != 0; }
  BL_INLINE bool isSynthesized() const noexcept { return hasFlag(kFlagSynthesized); }
  BL_INLINE bool isMinimum() const noexcept { return hasFlag(kFlagMinimum); }
  BL_INLINE bool isCrossStream() const noexcept { return hasFlag(kFlagCrossStream); }
  BL_INLINE bool isOverride() const noexcept { return hasFlag(kFlagOverride); }

  BL_INLINE const void* calcDataPtr(const void* basePtr) const noexcept {
    return isSynthesized() ? dataPtr : static_cast<const void*>(static_cast<const uint8_t*>(basePtr) + dataOffset);
  }

  static BL_INLINE KernGroup makeReferenced(uint32_t format, uint32_t flags, uintptr_t dataOffset, uint32_t dataSize) noexcept {
    return KernGroup { format, flags, dataSize, { dataOffset } };
  }

  static BL_INLINE KernGroup makeSynthesized(uint32_t format, int32_t flags, void* dataPtr, uint32_t dataSize) noexcept {
    return KernGroup { format, flags | kFlagSynthesized, dataSize, { (uintptr_t)dataPtr } };
  }
};

// ============================================================================
// [BLOpenType::KernCollection]
// ============================================================================

class KernCollection {
public:
  BL_NONCOPYABLE(KernCollection)

  BLArray<KernGroup> groups;

  BL_INLINE KernCollection() noexcept {}
  BL_INLINE ~KernCollection() noexcept { releaseData(); }

  BL_INLINE bool empty() const noexcept { return groups.empty(); }

  BL_INLINE void reset() noexcept {
    releaseData();
    groups.reset();
  }

  void releaseData() noexcept {
    size_t count = groups.size();
    for (size_t i = 0; i < count; i++) {
      const KernGroup& group = groups[i];
      if (group.isSynthesized())
        free(group.dataPtr);
    }
  }
};

// ============================================================================
// [BLOpenType::KernData]
// ============================================================================

//! Kerning data stored in `OTFace` and used to perform kerning.
class KernData {
public:
  BL_NONCOPYABLE(KernData)

  enum HeaderType : uint32_t {
    kHeaderWindows   = 0,
    kHeaderMac       = 1
  };

  BLFontTable table;
  uint8_t headerType;
  uint8_t headerSize;
  uint8_t reserved[6];
  KernCollection collection[2];

  BL_INLINE KernData() noexcept {}
};

// ============================================================================
// [BLOpenType::KernImpl]
// ============================================================================

namespace KernImpl {
  BLResult init(BLOTFaceImpl* faceI, const BLFontData* fontData) noexcept;
}

} // {BLOpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTKERN_P_H
