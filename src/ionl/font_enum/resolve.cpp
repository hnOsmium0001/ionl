#include "resolve.hpp"
#include "resolve_p.hpp"

#include <ionl/utils.hpp>

namespace {
using namespace FontEnum;

bool DoFontDescResolution(FontSet& out, const FamilyDescription& desc, Resolver resolver) {
#if defined(_WIN32)
    static DirectWriteContext ctxDWrite;
#elif defined(__APPLE__)
    // TODO
#elif defined(__linux__)
    // TODO
#endif

    if (resolver == Resolver::PlatformDefault) {
#if defined(_WIN32)
        resolver = Resolver::DirectWrite;
#elif defined(__APPLE__)
        resolver = Resolver::CoreText;
#elif defined(__linux__)
        resolver = Resolver::FontConfig;
#endif
    }

    switch (resolver) {
        using enum Resolver;
        case PlatformDefault: UNREACHABLE;

        case DirectWrite: {
#ifdef FONT_ENUM_DWRITE_RESOLVER_AVAIL
            for (std::string_view familyName : desc.familyNames) {
                auto it = ctxDWrite.familyNameMap.find(familyName);
                if (it != ctxDWrite.familyNameMap.end()) {
                    const DwFamily& fm = it->second;
                    for (size_t i = fm.begin; i < fm.end; ++i) {
                        const DwEnumeratedFont& f = ctxDWrite.enumeratedFonts[i];
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
