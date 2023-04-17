#pragma once

#include <dwrite.h>
#include <robin_hood.h>
#include <filesystem>
#include <string>
#include <vector>

namespace FontEnum::details {

struct DwEnumeratedFont {
    std::filesystem::path fontFile;
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
    std::vector<DwEnumeratedFont> foundFonts;
    robin_hood::unordered_map<std::string, DwFamily> familyNameMap;

    enum class InitResult {
        Success,
        FatalError,
    };
    InitResult Init() noexcept;
    ~DwCtx();

    void EnumSystemFonts();
} gDWriteCtx;

} // namespace FontEnum::details
