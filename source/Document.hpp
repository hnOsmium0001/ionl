#pragma once

#include <robin_hood.h>
#include <chrono>
#include <cstddef>
#include <deque>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Ionl {

/// Persistent bullet ID (saved to database)
/// This is currently the rowid in SQLite
using Pbid = size_t;
/// Runtime bullet ID (transient)
using Rbid = size_t;

// A number for `PRAGMA user_vesrion`, representing the current database version. Increment when the table format changes.
#define CURRENT_DATABASE_VERSION 1
// NOTE: macros for string literal concatenation only
#define ROOT_BULLET_PBID 1
#define ROOT_BULLET_RBID 0
constexpr Ionl::Pbid kRootBulletPbid = ROOT_BULLET_PBID;
constexpr Ionl::Rbid kRootBulletRbid = ROOT_BULLET_RBID;

/// NOTE: do not change these values, they are a part of the on-disk format
enum class BulletType {
    Textual = 1,
    Mirror = 2,
};

struct BulletContentTextual {
    std::string text;
};

struct BulletContentMirror {
    Pbid referee;
};

struct BulletContent {
    std::variant<
        BulletContentTextual,
        BulletContentMirror>
        v;

    BulletType GetType() const;
};

class Document;
struct Bullet {
    /* Document linked */ Document* document;
    /* Document linked */ Rbid rbid;
    Pbid pbid;
    Pbid parentPbid;
    // TODO do we actually want these two in memory? keeping them in sync with database is difficult
    // (requires passing extra data through BackingStore through every modification function)
    std::chrono::time_point<std::chrono::system_clock> creationTime;
    std::chrono::time_point<std::chrono::system_clock> modifyTime;
    BulletContent content;
    std::vector<Pbid> children;
    bool expanded = true;

    bool IsRootBullet() const;
};

class IBackingStore;
class Document {
private:
    IBackingStore* mStore;
    std::deque<std::optional<Bullet>> mBullets; // Index by bullet's rbid
    std::vector<size_t> mFreeRbids;
    robin_hood::unordered_flat_map<Pbid, Rbid> mPtoRmap;

public:
    Document(IBackingStore& store);

    Bullet& GetRoot();
    const Bullet& GetRoot() const;

    Bullet* GetBulletByRbid(Rbid rbid);
    Bullet* GetBulletByPbid(Pbid pbid);
    Bullet& FetchBulletByPbid(Pbid pbid);

    Bullet& CreateBullet();
    void DeleteBullet(Bullet& bullet);
    void UpdateBulletContent(Bullet& bullet);
    void ReparentBullet(Bullet& bullet, Bullet& newParent, int index);

private:
    Bullet* Store(Bullet bullet);
};

} // namespace Ionl