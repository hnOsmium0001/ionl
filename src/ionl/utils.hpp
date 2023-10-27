#pragma once

#include <robin_hood.h>
#include <utility>
#include <variant>

#define STRINGIFY_IMPL(text) #text
#define STRINGIFY(text) STRINGIFY_IMPL(text)

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_3(a, b, c) CONCAT(a, CONCAT(b, c))
#define CONCAT_4(a, b, c, d) CONCAT(CONCAT(a, b), CONCAT(c, d))

#define UNIQUE_NAME(prefix) CONCAT(prefix, __COUNTER__)
#define UNIQUE_NAME_LINE(prefix) CONCAT(prefix, __LINE__)
#define DISCARD UNIQUE_NAME(_discard)

#define UNUSED(x) (void)x;

#define PRINTF_STRING_VIEW(s) (int)s.size(), s.data()

#if defined(_MSC_VER)
#define UNREACHABLE __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE __builtin_unreachable()
#else
#define UNREACHABLE
#endif

#if _WIN32
#define PLATFORM_PATH_STR "%ls"
#else
#define PLATFORM_PATH_STR "%s"
#endif

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template <typename TVariant, typename... Ts>
auto VisitVariantOverloaded(TVariant&& v, Ts&&... cases) {
    return std::visit(Overloaded{ std::forward<Ts>(cases)... }, std::forward<TVariant>(v));
}

struct StringHash {
    using is_transparent = void;

    std::size_t operator()(const std::string& key) const { return robin_hood::hash_bytes(key.c_str(), key.size()); }
    std::size_t operator()(std::string_view key) const { return robin_hood::hash_bytes(key.data(), key.size()); }
    std::size_t operator()(const char* key) const { return robin_hood::hash_bytes(key, std::strlen(key)); }
};

struct StringEqual {
    using is_transparent = int;

    bool operator()(std::string_view lhs, const std::string& rhs) const {
        const std::string_view view = rhs;
        return lhs == view;
    }

    bool operator()(const char* lhs, const std::string& rhs) const {
        return std::strcmp(lhs, rhs.c_str()) == 0;
    }

    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs == rhs;
    }
};

template <typename TCleanupFunc>
class ScopeGuard {
private:
    TCleanupFunc mFunc;
    bool mDismissed = false;

public:
    /// Specifically left this implicit so that constructs like
    /// \code
    /// ScopeGuard sg = [&]() { res.Cleanup(); };
    /// \endcode
    /// would work. It is highly discourage and unlikely that one would want to use ScopeGuard as a function
    /// parameter, so the normal argument that implicit conversion are harmful doesn't really apply here.
    // Deliberately not explicit to allow usages like: ScopeGuard var = lambda;
    ScopeGuard(TCleanupFunc&& function) noexcept
        : mFunc{ std::move(function) } {
    }

    ~ScopeGuard() noexcept {
        if (!mDismissed) {
            mFunc();
        }
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& that) noexcept
        : mFunc{ std::move(that.mFunc) } {
        that.Cancel();
    }

    ScopeGuard& operator=(ScopeGuard&& that) noexcept {
        if (!mDismissed) {
            mFunc();
        }
        this->mFunc = std::move(that.mFunc);
        this->cancelled = std::exchange(that.cancelled, true);
    }

    void Dismiss() noexcept {
        mDismissed = true;
    }
};

template <typename T>
auto GuardDeletion(T* ptr) {
    return ScopeGuard([ptr]() {
        delete ptr;
    });
}

#define SCOPE_GUARD(name) ScopeGuard name = [&]()
#define DEFER ScopeGuard UNIQUE_NAME(scopeGuard) = [&]()
