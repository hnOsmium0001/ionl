#pragma once

#include <ionl/font_enum/resolve.hpp>
#include <ionl/utils.hpp>

#include <robin_hood.h>
#include <filesystem>
#include <string>
#include <vector>

#ifdef FONT_ENUM_DWRITE_RESOLVER_AVAIL
#define WIN32_LEAN_AND_MEAN
#include <dwrite.h>

namespace FontEnum {

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

struct DirectWriteContext {
    IDWriteFactory* factory = nullptr;
    std::vector<DwEnumeratedFont> enumeratedFonts;
    robin_hood::unordered_map<std::string, DwFamily, StringHash, StringEqual> familyNameMap;

    DirectWriteContext();
    ~DirectWriteContext();

    // Returns true if DirectWrite has failed to initialize and this backend is unavailable
    bool IsErrornous() const { return factory == nullptr; }

    void EnumSystemFonts();
    void ClearKnownFonts();
};

} // namespace FontEnum

#endif
