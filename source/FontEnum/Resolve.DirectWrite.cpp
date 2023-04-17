#include "Resolve.DirectWrite.hpp"

#include "../Utils.hpp"
#include "Resolve.hpp"

#include <wrl/client.h>
#include <cstdint>
#include <memory>

// Define `HRESULT hr;` at top of function to use this
#define TRY_HRESULT(exp) \
    hr = exp;            \
    if (FAILED(hr)) continue

using namespace FontEnum;
using namespace FontEnum::details;

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

template <typename T>
static void SafeRelease(T*& ptr) {
    if (ptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

// https://gist.github.com/xebecnan/6d070c93fb69f40c3673
std::string WcharToUTF8(const wchar_t* src, size_t srcLen) {
    if (!src) {
        return {};
    }

    if (srcLen == 0) {
        srcLen = wcslen(src);
    }

    int length = WideCharToMultiByte(CP_UTF8, 0, src, srcLen, nullptr, 0, nullptr, nullptr);
    std::string res(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, src, srcLen, res.data(), length, nullptr, nullptr);
    return res;
}

DwCtx::InitResult DwCtx::Init() noexcept {
    using enum InitResult;
    HRESULT hr;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&factory));
    if (FAILED(hr)) {
        return FatalError;
    }

    return Success;
}

DwCtx::~DwCtx() {
    SafeRelease(factory);
}

void DwCtx::EnumSystemFonts() {
    HRESULT hr;

    ComPtr<IDWriteFontCollection> sysFontCollection;
    hr = factory->GetSystemFontCollection(&sysFontCollection);
    if (FAILED(hr)) {
        return;
    }

    UINT32 fontFamilyCnt = sysFontCollection->GetFontFamilyCount();
    for (UINT32 i = 0; i < fontFamilyCnt; ++i) {
        ComPtr<IDWriteFontFamily> fontFamily;
        TRY_HRESULT(sysFontCollection->GetFontFamily(i, &fontFamily));

        size_t generatedBeginIdx = foundFonts.size();
        size_t generatedCnt = 0;
        UINT32 fontCnt = fontFamily->GetFontCount();
        for (UINT32 j = 0; j < fontCnt; ++j) {
            ComPtr<IDWriteFont> font;
            TRY_HRESULT(fontFamily->GetFont(j, &font));

            ComPtr<IDWriteFontFace> fontFace;
            TRY_HRESULT(font->CreateFontFace(&fontFace));

            // Ignore type1 fonts since our font stack can't handle them gracefully
            if (fontFace->GetType() == DWRITE_FONT_FACE_TYPE_TYPE1) {
                continue;
            }

            UINT32 fontFilesCnt;
            fontFace->GetFiles(/* query number of files available */ &fontFilesCnt, nullptr);

            auto fontFiles = std::make_unique<IDWriteFontFile*[]>(fontFilesCnt);
            TRY_HRESULT(fontFace->GetFiles(/* provide number of font files to request */ &fontFilesCnt, fontFiles.get()));
            DEFER {
                for (UINT32 k = 0; k < fontFilesCnt; ++k) {
                    fontFiles[k]->Release();
                }
            };

            // https://stackoverflow.com/questions/41161152/when-can-an-idwritefontface-have-more-than-one-file
            // According to this, a IDWriteFontFace can never have more than 1 file except in the case of Type 1 fonts which we filter out above already
            if (fontFilesCnt > 1) {
                // TODO log warning
            }
            for (UINT32 k = 0; k < fontFilesCnt; ++k) {
                IDWriteFontFile* fontFile = fontFiles[k];

                ComPtr<IDWriteFontFileLoader> loader;
                TRY_HRESULT(fontFile->GetLoader(&loader));

                // Check if the loader is backed by a local file, in which case there is a file available for us to query
                // TODO maybe we can fetch a TTF/OTF stream from the non-local font and feed it to Freetype directly as a memory blob?
                ComPtr<IDWriteLocalFontFileLoader> localLoader;
                TRY_HRESULT(loader.As<IDWriteLocalFontFileLoader>(&localLoader));

                const void* refKey;
                UINT32 refKeySize;
                TRY_HRESULT(fontFile->GetReferenceKey(&refKey, &refKeySize));

                UINT32 filePathLen;
                // Gets file path length without null terminator
                TRY_HRESULT(localLoader->GetFilePathLengthFromKey(refKey, refKeySize, &filePathLen));

                // Add 1 to account for null terminator since GetFilePathFromKey wants it
                auto filePath = std::make_unique<WCHAR[]>(filePathLen + 1);
                TRY_HRESULT(localLoader->GetFilePathFromKey(refKey, refKeySize, filePath.get(), filePathLen + 1));

                DwEnumeratedFont ef{
                    .fontFile = std::filesystem::path(filePath.get(), filePath.get() + filePathLen),
                    .style = font->GetStyle(),
                    .stretch = font->GetStretch(),
                    .weight = font->GetWeight(),
                };
                foundFonts.push_back(std::move(ef));
                generatedCnt += 1;
            }
        }

        if (generatedCnt > 0) {
            ComPtr<IDWriteLocalizedStrings> familyNames;
            TRY_HRESULT(fontFamily->GetFamilyNames(&familyNames));

            UINT32 index;
            BOOL exists;
            TRY_HRESULT(familyNames->FindLocaleName(L"en-us", &index, &exists));

            // If the specified locale doesn't exist, select the first on the list.
            if (!exists) {
                index = 0;
            }

            // Without null terminator
            UINT32 nameLen;
            TRY_HRESULT(familyNames->GetStringLength(index, &nameLen));

            auto nameCstr = std::make_unique<WCHAR[]>(nameLen + 1);
            TRY_HRESULT(familyNames->GetString(index, nameCstr.get(), nameLen + 1));

            familyNameMap.try_emplace(
                WcharToUTF8(nameCstr.get(), nameLen),
                DwFamily{ generatedBeginIdx, generatedBeginIdx + generatedCnt });
        }
    }
}
