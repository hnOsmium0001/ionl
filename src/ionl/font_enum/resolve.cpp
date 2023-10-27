#include "resolve.hpp"
#include "resolve_p.hpp"

#include <ionl/utils.hpp>

namespace FontEnum {

std::vector<UnloadedFont> FontEnum::Resolve(const FamilyDescription& desc) {
    std::vector<UnloadedFont> result;

#if defined(_WIN32)
    static DirectWriteContext ctxDWrite;
    for (std::string_view familyName : desc.familyNames) {
        auto it = ctxDWrite.familyNameMap.find(familyName);
        if (it != ctxDWrite.familyNameMap.end()) {
            const DwFamily& fm = it->second;
            for (size_t i = fm.begin; i < fm.end; ++i) {
                const DwEnumeratedFont& f = ctxDWrite.enumeratedFonts[i];
                result.push_back(f.representativeFont);
            }
        }
    }
#elif defined(__APPLE__)
    // TODO
#elif defined(__linux__)
    // TODO
#endif

    return result;
}

} // namespace FontEnum
