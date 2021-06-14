#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <boost/lockfree/queue.hpp>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using AvailableObjectsQueueType =
    boost::lockfree::queue<std::size_t, boost::lockfree::fixed_sized<true>>;

template <typename T>
class ObjectPool;

template <typename T>
class TObject
{
    T* obj;
    ObjectPool<T>* poolPtr;
    const std::size_t objectIndex;
    bool              returned = false;

    TObject() = delete;
    explicit TObject(T* Borrowed, ObjectPool<T>* PoolPtr, std::size_t ObjIndex)
        : obj(Borrowed), poolPtr(PoolPtr), objectIndex(ObjIndex)
    {
    }

public:
    friend ObjectPool<T>;
    void returnObj()
    {
        if (!returned && obj != nullptr) {
            poolPtr->availableObjectsQueue.push(objectIndex);
            if (poolPtr->availableObjectsCount < poolPtr->size())
                poolPtr->availableObjectsCount ++;
            returned = true;
        }
    }
    T* getObj() {
        return obj; 
    }
    ~TObject() {returnObj(); }
};

template <typename T>
class ObjectPool
{
    AvailableObjectsQueueType availableObjectsQueue;
    std::size_t               objectCount;
    std::vector<T>            objects;
    std::atomic_uint64_t      availableObjectsCount;
    std::atomic_bool          shutdownFlag;
    std::mutex                object_mutex;

public:
    friend TObject<T>;
    ObjectPool(std::size_t Size, std::function<T(std::size_t)> objectInitializers);
    ObjectPool() = delete;
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    TObject<T> acquire(uint64_t sleepTime_ms = 0);
    std::unique_ptr<TObject<T>> try_acquire(bool& success);
    std::size_t       size() const;
    uint64_t          getAvailableObjectsCount() const;
    std::vector<T>& getInternalObjects_unsafe();
    void              shutdown();
    ~ObjectPool();
};

template <typename T>
uint64_t ObjectPool<T>::getAvailableObjectsCount() const
{
    return availableObjectsCount.load();
}

template <typename T>
ObjectPool<T>::ObjectPool(std::size_t Size, std::function<T(std::size_t)> objectInitializers)
    : availableObjectsQueue(Size)
{
    shutdownFlag.store(false);
    for (std::size_t i = 0; i < Size; i++) {
        availableObjectsQueue.push(i);
    }
    objects.reserve(Size);
    for (std::size_t i = 0; i < Size; i++) {
        objects.push_back(objectInitializers(i));
    }
    objectCount = Size;
    availableObjectsCount.store(Size);
}

/// Obj can be null only when shutting down
template <typename T>
TObject<T> ObjectPool<T>::acquire(uint64_t sleepTime_ms)
{
    std::size_t currIndex = -1;
    bool        isShuttingDown = false;
    std::unique_lock<std::mutex> lck(this->object_mutex);
    while (!(isShuttingDown = shutdownFlag.load(std::memory_order_relaxed)) &&
        !availableObjectsQueue.pop(currIndex)) {
        if (sleepTime_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime_ms));
        }
    }
    if (!isShuttingDown) {
        availableObjectsCount--;
        TObject<T> res(&objects[currIndex], this, currIndex);
        assert(availableObjectsCount.load() >= 0);
        return res;
    }
    else {
        TObject<T> res(nullptr, this, currIndex);
        assert(availableObjectsCount.load() >= 0);
        return res;
    }
}

/// Obj can be null when shutting down or if the object is not available
template <typename T>
std::unique_ptr<TObject<T>> ObjectPool<T>::try_acquire(bool& success)
{
    std::size_t currIndex = -1;
    bool        isShuttingDown = shutdownFlag.load(std::memory_order_relaxed);
    if (!isShuttingDown && availableObjectsQueue.pop(currIndex)) {
        success = true;
    }
    else {
        success = false;
    }
    if (!isShuttingDown && success) {
        availableObjectsCount--;
        std::unique_ptr<TObject<T>> res;
        res.reset(new TObject<T>(&objects[currIndex], this, currIndex));
        assert(availableObjectsCount.load() >= 0);
        return std::move(res);
    }
    else {
        std::unique_ptr<TObject<T>> res;
        res.reset(new TObject<T>(nullptr, this, currIndex));
        assert(availableObjectsCount.load() >= 0);
        return std::move(res);
    }
}

template <typename T>
std::size_t ObjectPool<T>::size() const
{
    return objectCount;
}

template <typename T>
std::vector<T>& ObjectPool<T>::getInternalObjects_unsafe()
{
    return objects;
}

template <typename T>
void ObjectPool<T>::shutdown()
{
    // this can be called more than once
    shutdownFlag.store(true);
    while (availableObjectsCount != objectCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    availableObjectsQueue.consume_all([](std::size_t&) {});
    objects.clear();
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
    shutdown();
}

#endif // OBJECTPOOL_H
