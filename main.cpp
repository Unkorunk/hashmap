#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "hash_map.hpp"

TEST_CASE("allocate memory", "[test]") {
	fefu::allocator<int> tor;

	const int arr_size = 5;

	int* arr[arr_size];
	int sizes[arr_size];

	for (int i = 0; i < arr_size; i++) {
		sizes[i] = (i + 1) * (i + 1);
	}

	for (int i = 0; i < arr_size; i++) {
		arr[i] = tor.allocate(sizes[i]);
		for (int j = 0; j < sizes[i]; j++) {
			arr[i][j] = (i + 1) * (j + 1);
		}
	}

	for (int i = 0; i < arr_size; i++) {
		for (int j = 0; j < sizes[i]; j++) {
			REQUIRE(arr[i][j] == (i + 1) * (j + 1));
		}
	}

	for (int i = 0; i < arr_size; i++) {
		tor.deallocate(arr[i], 0);
	}
}