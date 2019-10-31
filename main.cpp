#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "hash_map.hpp"

TEST_CASE("allocate memory", "[test]") {
	fefu::allocator<int> tor;

	const int arr_size = 5;

	int* arr[arr_size + 3];
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

	tor.deallocate(arr[arr_size - 1], 1);
	arr[arr_size] = tor.allocate(2);
	tor.deallocate(arr[arr_size - 1], sizes[arr_size - 1]);
	arr[arr_size + 1] = tor.allocate(2);
	arr[arr_size + 2] = tor.allocate(3);
	tor.deallocate(arr[arr_size + 1], 2);
	tor.deallocate(arr[arr_size + 2], 3);
	arr[arr_size + 2] = tor.allocate(3);
	arr[arr_size + 1] = tor.allocate(2);
}