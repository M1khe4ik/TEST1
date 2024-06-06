// tests/test_kvstore.cpp

#include <gtest/gtest.h>
#include <unordered_map>
#include "../src/kvstore.h"

TEST(KVStoreTest, PutAndGet) {
    KVStore store;
    store.put("key1", "value1");
    ASSERT_EQ(store.get("key1"), "value1");
}

TEST(KVStoreTest, Del) {
    KVStore store;
    store.put("key1", "value1");
    ASSERT_EQ(store.del("key1"), "value1");
    ASSERT_EQ(store.get("key1"), "");
}

TEST(KVStoreTest, Count) {
    KVStore store;
    store.put("key1", "value1");
    store.put("key2", "value2");
    ASSERT_EQ(store.count(), 2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
