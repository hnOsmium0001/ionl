#pragma once

#include <ionl/document.hpp>

#include <memory>
#include <vector>

namespace Ionl {

class IBackingStore {
public:
    virtual ~IBackingStore() = default;
    virtual Bullet FetchBullet(Pbid pbid) = 0;
    virtual Pbid FetchParentOfBullet(Pbid bullet) = 0;
    virtual std::vector<Pbid> FetchChildrenOfBullet(Pbid bullet) = 0;
    virtual Pbid InsertEmptyBullet() = 0;
    virtual void DeleteBullet(Pbid bullet) = 0;
    virtual void SetBulletContent(Pbid bullet, const BulletContent& bulletContent) = 0;
    // TODO merge these two ops into one?
    virtual void SetBulletPositionAfter(Pbid bullet, Pbid newParent, Pbid relativeTo) = 0;
    virtual void SetBulletPositionAtBeginning(Pbid bullet, Pbid newParent) = 0;
};

class SQLiteBackingStore : public IBackingStore {
private:
    class Private;
    Private* m;

public:
    SQLiteBackingStore(const char* dbPath);
    ~SQLiteBackingStore();

    void BeginTransaction();
    void CommitTransaction();
    void RollbackTransaction();

    Bullet FetchBullet(Pbid pbid) override;
    Pbid FetchParentOfBullet(Pbid bullet) override;
    std::vector<Pbid> FetchChildrenOfBullet(Pbid bullet) override;
    Pbid InsertEmptyBullet() override;
    void DeleteBullet(Pbid bullet) override;
    void SetBulletContent(Pbid bullet, const BulletContent& bulletContent) override;
    void SetBulletPositionAfter(Pbid bullet, Pbid newParent, Pbid relativeTo) override;
    void SetBulletPositionAtBeginning(Pbid bullet, Pbid newParent) override;
};

class WriteDelayedBackingStore : public IBackingStore {
private:
    struct QueuedOperation;

    SQLiteBackingStore* mReceiver;
    std::vector<QueuedOperation> mQueuedOps;

public:
    WriteDelayedBackingStore(SQLiteBackingStore& receiver);
    ~WriteDelayedBackingStore();

    Bullet FetchBullet(Pbid pbid) override;
    Pbid FetchParentOfBullet(Pbid bullet) override;
    std::vector<Pbid> FetchChildrenOfBullet(Pbid bullet) override;
    Pbid InsertEmptyBullet() override;
    void DeleteBullet(Pbid bullet) override;
    void SetBulletContent(Pbid bullet, const BulletContent& bulletContent) override;
    void SetBulletPositionAfter(Pbid bullet, Pbid newParent, Pbid relativeTo) override;
    void SetBulletPositionAtBeginning(Pbid bullet, Pbid newParent) override;

    size_t GetUnflushedOpsCount() const;
    void ClearOps();
    void FlushOps();
};

} // namespace Ionl