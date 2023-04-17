#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

// NOTE: we wrap an internal structure instead of just some IResolver in order to avoid exposing the Resolve.*.hpp
//  headers to end users, because they include other intrusive headers like Windows.h

namespace FontEnum {

// Counterpart to CSS's font-family string
struct FontDescription {
    // TODO

    void Append(const FontDescription& otherDesc);
    void AppendFamily(std::string_view familyName);
};

// TODO handle subfamiles/opentype variants?
// TODO maybe it would be better if we didn't make a copy of a Font object very time we resolve for fonts
struct Font {
    std::string familyName;
    std::filesystem::path file;
};

struct FontSet {
    std::vector<Font> fonts;
};

enum class Resolver {
    PlatformDefault,
    DirectWrite,
    CoreText,
    FontConfig,
    Filesystem,
};

struct ResolutionPreferences {
    Resolver resolver = Resolver::PlatformDefault;
};

FontSet Resolve(const FontDescription& desc, const ResolutionPreferences& pref);

} // namespace FontEnum
