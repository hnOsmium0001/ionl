#pragma once

#include "Resolve.hpp"
#ifdef FONT_ENUM_DWRITE_RESOLVER_AVAIL

#include "../Utils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <dwrite.h>
#include <robin_hood.h>
#include <filesystem>
#include <string>
#include <vector>

namespace FontEnum::details {

struct DwEnumeratedFont {
    Font representativeFont;
    DWRITE_FONT_STYLE style;
    DWRITE_FONT_STRETCH stretch;
    DWRITE_FONT_WEIGHT weight;
};

/// Describes a range of DwEnumeratedFont's that belongs to a single family.
struct DwFamily {
    size_t begin;
    size_t end;
};

struct DwCtx {
    IDWriteFactory* factory = nullptr;
    std::vector<DwEnumeratedFont> enumeratedFonts;
    robin_hood::unordered_map<std::string, DwFamily, StringHash, StringEqual> familyNameMap;

    enum class InitResult {
        Success,
        FatalError,
    };
    InitResult Init() noexcept;
    ~DwCtx();

    void EnumSystemFonts();
    void ClearKnownFonts();
} ;

extern DwCtx gDwCtx;

} // namespace FontEnum::details

#else
#warning "Included Resolve.DirectWrite.hpp when this resolver is not implemented (disabled by FONT_ENUM_DWRITE_RESOLVER_AVAIL)."
#endif
