#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <synchapi.h>
#include "pool.h"
#include <assert.h>

#define TEST(a,b)
#define EXPECT_EQ(a__,b__) assert(a__ == b__);
#define EXPECT_NE(a__,b__) assert(a__ != b__);
#define EXPECT_TRUE(a__) assert(a__);
#define EXPECT_TRUE(a__) assert(a__);
#define EXPECT_FALSE(a__) assert(!a__);

int main() {
	TEST(SimplePoolTest, Filling)
	{
		Pool* pool;
		init_pool(&pool, 5, sizeof(int));
		int elem = 5;
		EXPECT_TRUE(1 == add_element(pool, &elem));
		elem = 4;
		EXPECT_TRUE(1 == add_element(pool, &elem));
		elem = 3;
		EXPECT_TRUE(1 == add_element(pool, &elem));
		elem = 2;
		EXPECT_TRUE(1 == add_element(pool, &elem));
		elem = 1;
		EXPECT_TRUE(1 == add_element(pool, &elem));
		elem = 0;
		EXPECT_TRUE(0 == add_element(pool, &elem));
		int* ret = get_last_elemnt(pool);
		EXPECT_EQ(1, *ret);
		ret = get_last_elemnt(pool);
		EXPECT_EQ(2, *ret);
		ret = get_last_elemnt(pool);
		EXPECT_EQ(3, *ret);
		ret = get_last_elemnt(pool);
		EXPECT_EQ(4, *ret);
		ret = get_last_elemnt(pool);
		EXPECT_EQ(5, *ret);
		del_pool(pool);
	}
 	return 0;
}