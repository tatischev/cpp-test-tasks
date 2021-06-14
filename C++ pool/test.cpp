#include "pch.h"
#include "ObjectPool.h"

TEST(ObjectPoolTest, StressTest) {
    std::size_t poolSize = 10;
    std::size_t numThreads = 10;

    std::vector<std::unique_ptr<std::thread>> threads(numThreads);

    const int       factor = 10;
    ObjectPool<int> p(poolSize, [&factor](std::size_t z) { return factor * z; });
    for (std::unique_ptr<std::thread>& t : threads) {
        t.reset(new std::thread([&p]() {
            auto o = p.acquire();
            *o.getObj() = *o.getObj() + 1;
            }));
    }

    for (std::unique_ptr<std::thread>& t : threads) {
        t->join();
    }

    int expectedSum = numThreads;
    int sum = 0;
    for (std::size_t i = 0; i < poolSize; i++) {
        expectedSum += factor * i;
        sum += *p.acquire().getObj();
    }
    EXPECT_EQ(sum, expectedSum);
}

TEST(ObjectPoolTest, TryBorrow) {
    std::size_t poolSize = 4;

    const int       factor = 10;
    ObjectPool<int> p(poolSize, [&factor](std::size_t z) { return factor * z; });
    bool            success = false;

    EXPECT_EQ(p.getAvailableObjectsCount(), 4);

    std::unique_ptr<TObject<int>> o1 = p.try_acquire(success);
    EXPECT_TRUE(success);
    EXPECT_EQ(p.getAvailableObjectsCount(), 3);
    EXPECT_NE(o1->getObj(), nullptr);

    std::unique_ptr<TObject<int>> o2 = p.try_acquire(success);
    EXPECT_TRUE(success);
    EXPECT_EQ(p.getAvailableObjectsCount(), 2);
    EXPECT_NE(o2->getObj(), nullptr);

    std::unique_ptr<TObject<int>> o3 = p.try_acquire(success);
    EXPECT_TRUE(success);
    EXPECT_EQ(p.getAvailableObjectsCount(), 1);
    EXPECT_NE(o3->getObj(), nullptr);

    std::unique_ptr<TObject<int>> o4 = p.try_acquire(success);
    EXPECT_TRUE(success);
    EXPECT_EQ(p.getAvailableObjectsCount(), 0);
    EXPECT_NE(o4->getObj(), nullptr);

    std::unique_ptr<TObject<int>> o5 = p.try_acquire(success);
    EXPECT_FALSE(success);
    EXPECT_EQ(o5->getObj(), nullptr);

    o5->returnObj();
    EXPECT_EQ(p.getAvailableObjectsCount(), 0);

    o4->returnObj();
    EXPECT_EQ(p.getAvailableObjectsCount(), 1);

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
