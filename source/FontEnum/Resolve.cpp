#include "Resolve.hpp"

#include "Resolve.DirectWrite.hpp"
#include "Resolve.Filesystem.hpp"

namespace {
using namespace FontEnum;
using namespace FontEnum::details;

bool DoFontDescResolution(FontSet& out, const FamilyDescription& desc, Resolver resolver) {
    if (resolver == Resolver::PlatformDefault) {
#if defined(_WIN32)
        resolver = Resolver::DirectWrite;
#elif defined(__APPLE__)
        resolver = Resolver::CoreText;
#elif defined(__linux__)
        resolver = Resolver::FontConfig;
#else
        resolver = Resolver::Filesystem;
#endif
    }

    switch (resolver) {
        using enum Resolver;
        case PlatformDefault: UNREACHABLE;

        case DirectWrite: {
#ifdef FONT_ENUM_DWRITE_RESOLVER_AVAIL
            for (std::string_view familyName : desc.familyNames) {
                auto it = gDwCtx.familyNameMap.find(familyName);
                if (it != gDwCtx.familyNameMap.end()) {
                    const DwFamily& fm = it->second;
                    for (size_t i = fm.begin; i < fm.end; ++i) {
                        const DwEnumeratedFont& f = gDwCtx.enumeratedFonts[i];
                        out.fonts.push_back(&f.representativeFont);
                    }
                }
            }
            return true;
#else
            return false;
#endif
        }

        case CoreText: {
#ifdef FONT_ENUM_CORETEXT_RESOLVER_AVAIL
            // TODO
#else
            return false;
#endif
        }

        case FontConfig: {
#ifdef FONT_ENUM_FONTCONFIG_RESOLVER_AVAIL
            // TODO
#else
            return false;
#endif
        }

        case Filesystem: {
#ifdef FONT_ENUM_FS_RESOLVER_AVAIL
            // TODO
#else
            return false;
#endif
        }
    }

    return false;
}
} // namespace

auto FontEnum::Resolve(const FamilyDescription& desc, const ResolutionPreferences& pref) -> FontSet {
    bool useFallback = pref.primaryResolver != pref.fallbackResolver;
    FontSet result;
    bool primaryPassSuccess = DoFontDescResolution(result, desc, pref.primaryResolver);
    if (!primaryPassSuccess && useFallback) {
        bool fallbackPassSuccess = DoFontDescResolution(result, desc, pref.fallbackResolver);
        // TODO do we want to report this error?
        (void)fallbackPassSuccess;
    }
    return result;
}
