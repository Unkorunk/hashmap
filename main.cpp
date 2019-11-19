#include <string>
#include <iostream>
#include <cassert>

#include "hash_map.hpp"

int main() {
	{
		struct TestStruct {
			int x, y;
			TestStruct() : x(0), y(0) {}
			TestStruct(int x, int y) : x(x), y(y) {}
		};

		fefu::hash_map<std::string, TestStruct> some_hash_map_1;
		some_hash_map_1.try_emplace(std::string("abc"), 56, 28);
		some_hash_map_1.try_emplace(std::string("abc1"), 56, 28);
		for (int i = 4; i < 1000; i++) {
			some_hash_map_1.try_emplace(std::string("abc2") + std::to_string(i), rand() % 32, rand() % 32);
		}

		std::cout << "Buckets: " << some_hash_map_1.bucket_count() << std::endl;
		std::cout << "Length: " << some_hash_map_1.size() << std::endl;
		for (auto iter = some_hash_map_1.begin(); iter != some_hash_map_1.end(); iter++) {
			std::cout << iter->first << ": (" << iter->second.x << ", " << iter->second.y << ")" << std::endl;
		}

		std::cout << some_hash_map_1["abc"].x << std::endl;
		std::cout << some_hash_map_1["abc"].y << std::endl;
	}

	{
		fefu::hash_map<std::string, int> ages(10);
		ages.emplace("Some name", 1);
		ages.try_emplace(std::string("Some name1"), 2);
		std::string some_name3 = "Some name 2";
		ages.try_emplace(some_name3, 3);
		some_name3 += "1";
		ages.try_emplace(some_name3, 1234.011f);

		std::string name = "Vlad";
		ages[name] = 19;
		name = "Kolya";
		ages[name] = 18;

		ages["Anya"] = 20;

		assert(ages[name] == 18);
		name = "Vlad";
		assert(ages[name] == 19);

		assert(ages.at("Kolya") == 18);
		try {
			ages.at("Katya");
			assert(false);
		} catch (const std::out_of_range & ) {
		}

		assert(ages["Anya"] == 20);

		assert(ages.contains("Vlad"));
		assert(ages.contains("Kolya"));
		assert(!ages.contains("Kolyas"));


		assert(ages.contains("Some name") && ages["Some name"] == 1);
		assert(ages.contains("Some name1") && ages["Some name1"] == 2);
		assert(ages.contains("Some name 2") && ages["Some name 2"] == 3);
		assert(ages.contains("Some name 21") && ages["Some name 21"] == 1234);

		int counter = 0;
		for (auto iter = ages.begin(); iter != ages.end(); iter++) {
			std::cout << iter->first << ": " << iter->second << std::endl;
			iter->second = counter++;
		}

		ages.insert(std::pair<std::string, int>{ "rvalue refrence", 50 });
		std::pair<const std::string, int> some_data = std::pair<const std::string, int>("lvalue reference", 12345);
		ages.insert(some_data);
		assert(some_data.second == 12345);


		for (auto iter = ages.begin(); iter != ages.end(); iter++) {
			std::cout << iter->first << ": " << iter->second << std::endl;
		}
	}

	{
		std::initializer_list<std::pair<const int, int>> data = {
			{73, 12345}, {119, 4567653}, {1, 2},   {2, 1},
			{50, 1},     {13466, 25},    {43, 25}, {29, 456} };

		// for (int i = 0; i < INT_MAX; i++) {
		//	if (i == 2) continue;

		//	if (std::hash<int>()(2) % 30 == std::hash<int>()(i) % 30) {
		//		std::cout << "Collision " << i << std::endl;
		//	}
		//}

		const fefu::hash_map<int, int> some_data(data, 30);

		assert(some_data.bucket_count() == 30);

		for (auto& vls : data) {
			assert(some_data.at(vls.first) == vls.second);
			assert(some_data.find(vls.first) != some_data.end() && some_data.find(vls.first)->second == vls.second &&
				some_data.count(vls.first) == 1);
		}

		for (int i = 0; i < 2000; i++) {
			bool cont = false;
			int rnd_num = rand() % 2000;
			for (auto iter = data.begin(); iter != data.end(); iter++) {
				if (iter->first == rnd_num) {
					cont = true;
					break;
				}
			}
			if (!cont) {
				assert(some_data.find(rnd_num) == some_data.end());
				assert(some_data.count(rnd_num) == 0);
			} else {
				assert(some_data.find(rnd_num) != some_data.end());
				assert(some_data.count(rnd_num) == 1);
			}
		}

		for (auto iter = some_data.begin(); iter != some_data.end(); iter++) {
			std::cout << iter->first << ": " << iter->second << std::endl;
		}
	}

	{
		fefu::hash_map<int, int> test_merge_1 { 
			{1, 2},
			{5, 1},
			{3, 7},
			{2, 6}
		};

		fefu::hash_map<int, int> test_merge_2{
			{4, 2},
			{1, 7},
			{6, 1},
			{2, 6}
		};

		test_merge_1.merge(test_merge_2);
		std::initializer_list<std::pair<const int, int>> after_merge_1 = {
			{1, 2},
			{5, 1},
			{3, 7},
			{2, 6},
			{4, 2},
			{6, 1}
		};

		std::initializer_list<std::pair<const int, int>> after_merge_2 = {
			{1, 7},
			{2, 6}
		};

		for (auto iter = after_merge_1.begin(); iter != after_merge_1.end(); iter++) {
			assert(test_merge_1.contains(iter->first) && test_merge_1[iter->first] == iter->second);
		}

		for (auto iter = after_merge_2.begin(); iter != after_merge_2.end(); iter++) {
			assert(test_merge_2.contains(iter->first) && test_merge_2[iter->first] == iter->second);
		}
	}

	std::cout << "OK" << std::endl;

	return 0;
}
