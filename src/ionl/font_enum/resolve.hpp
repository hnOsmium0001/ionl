#pragma once

// TODO "blob resolver" that lookups from a list of fonts compiled into the binary?
//      have some kind of comp-time registration mechanism by declaring byte arrays or something

#ifdef _WIN32
#define FONT_ENUM_DWRITE_RESOLVER_AVAIL
#endif

#ifdef __APPLE__
#define FONT_ENUM_CORETEXT_RESOLVER_AVAIL
#endif

#ifdef __linux__
#define FONT_ENUM_FONTCONFIG_RESOLVER_AVAIL
#endif

#include <filesystem>
#include <string_view>
#include <vector>
#include <span>

// NOTE: we wrap an internal structure instead of just some IResolver in order to avoid exposing the Resolve.*.hpp
//  headers to end users, because they include other intrusive headers like Windows.h

namespace FontEnum {

enum class FontFileType {
    // .ttf files, or .otf files with TrueType outlines
    TrueType,
    // .otf files with CFF outlines
    CFF,
    // NOTE: Even though this corresponds to the ".ttc" file extension, realistically it's used for the
    // collection format specified by CFF (which evolved from the original TrueType Collection specs),
    // and is compatible with all sorts of outlines. The "TrueType" in the name is just to correspond to
    // the file extension.
    TrueTypeCollection,
    // To deal with the other file types supported by various APIs. If there is ever a font that we want
    // to handle, it will always have an explicit entry above.
    Unknown,
};

// Counterpart to CSS's font-family string
// TODO do we want to support an owning version of this? i.e. std::vector<std::string>
struct FamilyDescription {
    std::span<std::string_view> familyNames;
};

// TODO handle subfamiles/opentype variants?
struct UnloadedFont {
    std::string familyName;
    std::filesystem::path file;
    FontFileType fileType;
};

std::vector<UnloadedFont> Resolve(const FamilyDescription& desc);

} // namespace FontEnum
