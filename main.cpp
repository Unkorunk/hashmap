#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <climits>
#include <string>
#include <set>

#include "hash_map.hpp"

using namespace std;
using fefu::hash_map;

#define EPS 1e-9

TEST_CASE("constructor(int n)", "[constructors]") {
	hash_map<int, int> hm1;

	REQUIRE(hm1.size() == 0);
	REQUIRE(hm1.bucket_count() == 1);
	REQUIRE(abs(hm1.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm1.load_factor()) < EPS);

	hm1.insert({ 1, 2 });
	REQUIRE(hm1.contains(1));

	REQUIRE(hm1.size() == 1);
	REQUIRE(hm1.bucket_count() == 1);
	REQUIRE(abs(hm1.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm1.load_factor() - 1.0f) < EPS);

	hm1.insert({ 2, 3 });
	REQUIRE(hm1.contains(1));
	REQUIRE(hm1.contains(2));
	hm1.erase(1);
	REQUIRE(!hm1.contains(1));

	REQUIRE(hm1.size() == 1);
	REQUIRE(hm1.bucket_count() == 2);
	REQUIRE(abs(hm1.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm1.load_factor() - 0.5f) < EPS);
}

TEST_CASE("copy constructor", "[constructors]") {
	hash_map<int, int> hm1{ {1, 2}, {2, 3} };
	hash_map<int, int> hm2(hm1);

	for (auto iter = hm1.begin(); iter != hm1.end(); iter++) {
		auto iter2 = hm2.find(iter->first);
		REQUIRE((iter2 != hm2.end() && *iter2 == *iter));
	}

	hm1.emplace(3, 6);
	REQUIRE((hm1.contains(3) && !hm2.contains(3)));
}

TEST_CASE("move constructor", "[constructors]") {
	initializer_list<pair<const int, int>> il = { {1, 2}, {2, 3} };
	hash_map<int, int> hm1(il);
	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	hash_map<int, int> hm2(std::move(hm1));

	REQUIRE(hm1.size() == 0);
	REQUIRE(hm1.bucket_count() == 0);

	REQUIRE(hm2.size() == 2);
	REQUIRE(hm2.bucket_count() == 4);
	REQUIRE(abs(hm2.max_load_factor() - 0.45f) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm2.find(iter->first);
		REQUIRE((iter2 != hm2.end() && *iter == *iter2));
	}
}

TEST_CASE("constructor(allocator)", "[constructors]") {
	initializer_list<pair<const int, int>> il = { {1, 2}, {2, 3}, {3, 6} };

	auto fefu_allocator = fefu::allocator<pair<const int, int>>();
	hash_map<int, int> hm1(fefu_allocator);
	auto stl_allocator = std::allocator<pair<const int, int>>();
	hash_map<int, int, std::hash<int>, std::equal_to<int>, std::allocator<pair<const int, int>>> hm2(stl_allocator);

	REQUIRE((typeid(hm1.get_allocator()).hash_code() == typeid(fefu_allocator).hash_code()));
	REQUIRE((typeid(hm2.get_allocator()).hash_code() == typeid(stl_allocator).hash_code()));
	REQUIRE((typeid(hm1.get_allocator()).hash_code() != typeid(hm2.get_allocator()).hash_code()));

	hm1.insert(il);

	REQUIRE(hm1.size() == 3);
	REQUIRE(hm1.bucket_count() == 4);
	REQUIRE(abs(hm1.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm1.load_factor() - 3.0f / 4.0f) < EPS);

	hm2.insert(il);

	REQUIRE(hm2.size() == 3);
	REQUIRE(hm2.bucket_count() == 4);
	REQUIRE(abs(hm2.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm2.load_factor() - 3.0f / 4.0f) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter1 = hm1.find(iter->first);
		auto iter2 = hm2.find(iter->first);

		REQUIRE((iter2 != hm2.end() && iter1 != hm1.end() && *iter1 == *iter2 && *iter2 == *iter));
	}
}

TEST_CASE("copy constructor(data, allocator)", "[constructors]") {
	initializer_list<pair<const int, int>> il = { {1, 2}, {2, 3}, {3, 6} };

	auto fefu_allocator = fefu::allocator<pair<const int, int>>(1);
	hash_map<int, int> hm1(fefu_allocator);
	hm1.insert(il.begin(), il.end());
	REQUIRE(hm1.size() == 3);
	REQUIRE(hm1.bucket_count() == 4);
	REQUIRE(abs(hm1.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm1.load_factor() - 3.0f / 4.0f) < EPS);

	auto other_fefu_allocator = fefu::allocator<pair<const int, int>>(2);
	hash_map<int, int, std::hash<int>, std::equal_to<int>, fefu::allocator<pair<const int, int>>> hm2(hm1, other_fefu_allocator);
	REQUIRE((typeid(hm1.get_allocator()).hash_code() == typeid(fefu_allocator).hash_code()));
	REQUIRE((typeid(hm2.get_allocator()).hash_code() == typeid(other_fefu_allocator).hash_code()));
	REQUIRE((hm1.get_allocator().unused_prop == 1 && hm2.get_allocator().unused_prop == 2));

	REQUIRE(hm2.size() == 3);
	REQUIRE(hm2.bucket_count() == 4);
	REQUIRE(abs(hm2.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm2.load_factor() - 3.0f / 4.0f) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter1 = hm1.find(iter->first);
		auto iter2 = hm2.find(iter->first);

		REQUIRE((iter2 != hm2.end() && iter1 != hm1.end() && *iter1 == *iter2 && *iter2 == *iter));
	}
}

TEST_CASE("move constructor(data, allocator)", "[constructors]") {
	initializer_list<pair<const int, int>> il = { {1, 2}, {2, 3}, {3, 6} };

	auto fefu_allocator = fefu::allocator<pair<const int, int>>(1);
	hash_map<int, int> hm1(fefu_allocator);
	hm1.insert(il.begin(), il.end());
	hm1.max_load_factor(0.4f);
	REQUIRE(hm1.size() == 3);
	REQUIRE(hm1.bucket_count() == 4);
	REQUIRE(abs(hm1.max_load_factor() - 0.4f) < EPS);
	REQUIRE(abs(hm1.load_factor() - 3.0f / 4.0f) < EPS);

	auto other_fefu_allocator = fefu::allocator<pair<const int, int>>(2);
	hash_map<int, int, std::hash<int>, std::equal_to<int>, fefu::allocator<pair<const int, int>>> hm2(std::move(hm1), other_fefu_allocator);
	REQUIRE((typeid(hm1.get_allocator()).hash_code() == typeid(fefu_allocator).hash_code()));
	REQUIRE((typeid(hm2.get_allocator()).hash_code() == typeid(other_fefu_allocator).hash_code()));
	REQUIRE((typeid(hm1.get_allocator()).hash_code() == typeid(hm2.get_allocator()).hash_code()));
	REQUIRE(hm2.get_allocator().unused_prop == 2);

	REQUIRE(hm2.size() == 3);
	REQUIRE(hm2.bucket_count() == 4);
	REQUIRE(abs(hm2.max_load_factor() - 0.4f) < EPS);
	REQUIRE(abs(hm2.load_factor() - 3.0f / 4.0f) < EPS);

	REQUIRE(hm1.size() == 0);
	REQUIRE(hm1.bucket_count() == 0);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm2.find(iter->first);

		REQUIRE((iter2 != hm2.end() && *iter == *iter2));
	}
}

TEST_CASE("constructor(init list)", "[constructors]") {
	initializer_list<pair<const int, string>> il = { {19, "Vlad"}, {5, "Yuria"}, {2, "Maria"}, {86, "Joni"} };
	hash_map<int, std::string> hm1(il, 1);

	REQUIRE(hm1.size() == 4);
	REQUIRE(hm1.bucket_count() == 8);
	REQUIRE(abs(hm1.load_factor() - 0.5) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);

		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	hash_map<int, std::string> hm2(il, 100);

	REQUIRE(hm2.size() == 4);
	REQUIRE(hm2.bucket_count() == 100);
	REQUIRE(abs(hm2.load_factor() - 4.0 / 100.0) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm2.find(iter->first);

		REQUIRE((iter2 != hm2.end() && *iter == *iter2));
	}
}

TEST_CASE("constructor(iter1, iter2)", "[constructors]") {
	initializer_list<pair<const int, string>> il = { {19, "Vlad"}, {5, "Yuria"}, {2, "Maria"}, {86, "Joni"} };
	hash_map<int, std::string> hm1(il.begin(), il.end());

	REQUIRE(hm1.size() == 4);
	REQUIRE(hm1.bucket_count() == 8);
	REQUIRE(abs(hm1.load_factor() - 0.5) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);

		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	set<pair<const string, int>> sinit = {
		{"Vlad", 19}, {"Yuria", 5}, {"Maria", 2}, {"Joni", 86}
	};

	hash_map<std::string, int> hm2(sinit.begin(), sinit.end());

	REQUIRE(hm2.size() == 4);
	REQUIRE(hm2.bucket_count() == 8);
	REQUIRE(abs(hm2.load_factor() - 0.5) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);

		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
}

TEST_CASE("copy assigment", "[assigment]") {
	hash_map<int, int> hm1{ {1, 2}, {2, 3} };
	hash_map<int, int> hm2;
	hm2 = hm1;

	for (auto iter = hm1.begin(); iter != hm1.end(); iter++) {
		auto iter2 = hm2.find(iter->first);
		REQUIRE((iter2 != hm2.end() && *iter2 == *iter));
	}

	hm1.emplace(3, 6);
	REQUIRE((hm1.contains(3) && !hm2.contains(3)));
}

TEST_CASE("move assigment", "[assigment]") {
	initializer_list<pair<const int, int>> il = { {1, 2}, {2, 3} };
	hash_map<int, int> hm1(il);
	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	hash_map<int, int> hm2;
	hm2 = std::move(hm1);

	REQUIRE(hm1.size() == 0);
	REQUIRE(hm1.bucket_count() == 0);

	REQUIRE(hm2.size() == 2);
	REQUIRE(hm2.bucket_count() == 4);
	REQUIRE(abs(hm2.max_load_factor() - 0.45f) < EPS);
	REQUIRE(abs(hm2.load_factor() - 0.5f) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm2.find(iter->first);
		REQUIRE((iter2 != hm2.end() && *iter == *iter2));
	}
}

TEST_CASE("assigment = init list", "[assigment]") {
	initializer_list<pair<const int, string>> il = { {19, "Vlad"}, {5, "Yuria"}, {2, "Maria"}, {86, "Joni"} };
	hash_map<int, std::string> hm1 = { {0, "None"}  };
	hm1 = il;

	REQUIRE(hm1.size() == 4);
	REQUIRE(hm1.bucket_count() == 8);
	REQUIRE(abs(hm1.load_factor() - 0.5) < EPS);

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);

		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	REQUIRE(!hm1.contains(0));
}

TEST_CASE("max_size", "[getters]") {
	hash_map<int, std::string> hm1;
	REQUIRE(hm1.max_size() == std::numeric_limits<size_t>().max());
}

TEST_CASE("begin", "[getters]") {
	hash_map<int, std::string> hm1;
	REQUIRE(hm1.begin() == hm1.end());
	hm1 = { {0, "MONE"} };
	REQUIRE((hm1.begin() != hm1.end() && hm1.begin()->first == 0 && hm1.begin()->second == "MONE"));
}

TEST_CASE("end", "[getters]") {
	hash_map<int, std::string> hm1;
	REQUIRE(hm1.end() == hm1.begin());
	hm1 = { {0, "MONE"} };
	REQUIRE((hm1.begin() != hm1.end() && hm1.begin()->first == 0 && hm1.begin()->second == "MONE" && ++hm1.begin() == hm1.end()));
}

TEST_CASE("cbegin", "[getters]") {
	const hash_map<int, std::string> hm2;
	REQUIRE(hm2.cbegin() == hm2.cend());
	const hash_map<int, std::string> hm1 = { {0, "MONE"} };
	REQUIRE((hm1.cbegin() != hm1.cend() && hm1.cbegin()->first == 0 && hm1.cbegin()->second == "MONE"));
}

TEST_CASE("cend", "[getters]") {
	const hash_map<int, std::string> hm2;
	REQUIRE(hm2.cend() == hm2.cbegin());
	const hash_map<int, std::string> hm1 = { {0, "MONE"} };
	REQUIRE((hm1.cbegin() != hm1.cend() && hm1.cbegin()->first == 0 && hm1.cbegin()->second == "MONE" && ++hm1.cbegin() == hm1.cend()));
}

TEST_CASE("const begin", "[getters]") {
	const hash_map<int, std::string> hm2;
	REQUIRE(hm2.begin() == hm2.end());
	const hash_map<int, std::string> hm1 = { {0, "MONE"} };
	REQUIRE((hm1.begin() != hm1.end() && hm1.begin()->first == 0 && hm1.begin()->second == "MONE"));
}

TEST_CASE("const end", "[getters]") {
	const hash_map<int, std::string> hm2;
	REQUIRE(hm2.end() == hm2.begin());
	const hash_map<int, std::string> hm1 = { {0, "MONE"} };
	REQUIRE((hm1.begin() != hm1.end() && hm1.begin()->first == 0 && hm1.begin()->second == "MONE" && ++hm1.begin() == hm1.end()));
}

TEST_CASE("emplace", "[emplace]") {
	initializer_list<pair<const int, string>> il = { {19, "Vlad"}, {5, "Yuria"}, {2, "Maria"}, {86, "Joni"} };

	hash_map<int, string> hm1;
	for (auto iter = il.begin(); iter != il.end(); iter++) {
		hm1.emplace(iter->first, iter->second);
	}

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter2 == *iter));
	}
}

TEST_CASE("try_emplace copy", "[emplace]") {
	set<pair<const int, pair<int, int>>> il;
	for (int i = 0; i < 100; i++) {
		il.emplace(i, pair<int, int>(rand(), rand()));
	}

	hash_map<int, pair<int, int>> hm1;
	for (auto iter = il.begin(); iter != il.end(); iter++) {
		hm1.try_emplace(iter->first, iter->second.first, iter->second.second);
	}

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter2 == *iter));
	}
}


TEST_CASE("try_emplace move", "[emplace]") {
	struct pair_hash {
		std::size_t operator() (const std::pair<int, int>& pair) const {
			return std::hash<int>()(pair.first);
		}
	};

	set<pair<const pair<int, int>, pair<int, int>>> il;
	for (int i = 0; i < 100; i++) {
		il.emplace(pair<int, int>(rand(), rand()), pair<int, int>(rand(), rand()));
	}

	hash_map<pair<int, int>, pair<int, int>, pair_hash> hm1;
	for (auto iter = il.begin(); iter != il.end(); iter++) {
		hm1.try_emplace(pair<int, int>(iter->first.first, iter->first.second), iter->second.first, iter->second.second);
	}

	for (auto iter = il.begin(); iter != il.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter2 == *iter));
	}
}

TEST_CASE("insert or assign copy", "[insert_or_assign]") {
	initializer_list<pair<const int, int>> ww = {
		{2, 5},
		{3, 6},
		{7, 9},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1;
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		hm1.insert_or_assign(iter->first, iter->second);
	}

	for (auto iter = ww2.begin(); iter != ww2.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	REQUIRE((hm1.size() == ww2.size()));
}

TEST_CASE("insert or assign move", "[insert_or_assign]") {
	initializer_list<pair<const int, int>> ww = {
		{2, 5},
		{3, 6},
		{7, 9},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1;
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		hm1.insert_or_assign(int(iter->first), iter->second);
	}

	for (auto iter = ww2.begin(); iter != ww2.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	REQUIRE((hm1.size() == ww2.size()));
}

TEST_CASE("erase on const iterator", "[erase]") {
	initializer_list<pair<const int, int>> ww = {
		{366, 5},
		{3, 6},
		{10, 9},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1(ww);
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	hm1.erase(fefu::hash_map_const_iterator<pair<const int, int>>(hm1.find(366)));
	CHECK_THROWS(hm1.erase(fefu::hash_map_const_iterator<pair<const int, int>>(hm1.find(228))));
	hm1.erase(fefu::hash_map_const_iterator<pair<const int, int>>(hm1.find(10)));
	for (auto iter = ww2.begin(); iter != ww2.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	REQUIRE((ww2.size() == hm1.size()));
}

TEST_CASE("erase on iterator", "[erase]") {
	initializer_list<pair<const int, int>> ww = {
		{366, 5},
		{3, 6},
		{10, 9},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1(ww);
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	hm1.erase(hm1.find(366));
	CHECK_THROWS(hm1.erase(hm1.find(228)));
	hm1.erase(hm1.find(10));
	for (auto iter = ww2.begin(); iter != ww2.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	REQUIRE((ww2.size() == hm1.size()));
}

TEST_CASE("erase element", "[erase]") {
	initializer_list<pair<const int, int>> ww = {
		{366, 5},
		{3, 6},
		{10, 9},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1(ww);
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	REQUIRE(hm1.erase(366) == 1);
	REQUIRE(hm1.erase(10) == 1);
	REQUIRE(hm1.erase(228) == 0);
	
	for (auto iter = ww2.begin(); iter != ww2.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	REQUIRE((ww2.size() == hm1.size()));
}

TEST_CASE("erase range", "[erase]") {
	initializer_list<pair<const int, int>> ww = {
		{10, 9},
		{366, 5},
		{3, 6},
		{2, 1},
		{7, 8}
	};

	initializer_list<pair<const int, int>> ww2 = {
		{3, 6},
		{2, 1},
		{7, 8}
	};

	hash_map<int, int> hm1(ww);
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}

	auto str = hm1.begin();
	auto str2 = next(hm1.begin(), 2);
	REQUIRE(str2 == hm1.erase(str, str2));

	REQUIRE((ww2.size() == hm1.size()));

	fefu::hash_map<int, int> hm3 = { {1, 2}, { 2, 3 } };
	REQUIRE(hm3.erase(hm3.end(), hm3.end()) == hm3.end());
}

TEST_CASE("clear", "[clear]") {
	initializer_list<pair<const int, hash_map<int, int>>> ww = {
		{10, {}},
		{366, {}},
		{3, {}},
		{2, {}},
		{7, {}}
	};

	hash_map<int, hash_map<int, int>> hm1(ww);
	for (auto iter = ww.begin(); iter != ww.end(); iter++) {
		auto iter2 = hm1.find(iter->first);
		REQUIRE((iter2 != hm1.end() && *iter == *iter2));
	}
	REQUIRE(hm1.size() == ww.size());
	
	hm1.clear();
	REQUIRE(hm1.size() == 0);
	REQUIRE(hm1.bucket_count() == 10);
}

TEST_CASE("get hash func", "[getters]") {
	hash_map<int, int, std::hash<int>> hm1;
	REQUIRE(typeid(hm1.hash_function()).hash_code() == typeid(std::hash<int>{}).hash_code());
	REQUIRE(typeid(hm1.hash_function()).hash_code() != typeid(std::hash<unsigned int>{}).hash_code());
}

TEST_CASE("get pred func", "[getters]") {
	hash_map<int, int> hm1;
	REQUIRE(typeid(hm1.key_eq()).hash_code() == typeid(std::equal_to<int>{}).hash_code());
	REQUIRE(typeid(hm1.key_eq()).hash_code() != typeid(std::equal_to<unsigned int>{}).hash_code());
}

TEST_CASE("merge copy", "[merge]") {
	hash_map<int, string> hm1 = { { 0, "test0" },  {1, "test1" } };
	hash_map<int, string> hm2 = { { 0, "test0" }, { 2, "test2" } };
	hm1.merge(hm2);
	REQUIRE(hm1.size() == 3);
	REQUIRE(hm2.size() == 1);

	CHECK(hm1.at(0) == "test0");
	CHECK(hm1.at(1) == "test1");
	CHECK(hm1.at(2) == "test2");

	CHECK(hm2.at(0) == "test0");
}

TEST_CASE("merge move", "[merge]") {
	hash_map<int, string> hm1 = { { 0, "test0" },  {1, "test1" }, { 2, "test2" } };
	hash_map<int, string> hm2 = { { 1, "test1" }, { 3, "test3" }, { 4, "test4" } };
	hm1.merge(std::move(hm2));
	REQUIRE(hm1.size() == 5);
	CHECK(hm1.at(0) == "test0");
	CHECK(hm1.at(1) == "test1");
	CHECK(hm1.at(2) == "test2");
	CHECK(hm1.at(3) == "test3");
	CHECK(hm1.at(4) == "test4");
}

TEST_CASE("bucket", "[bucket]") {
	hash_map<int, string> hm1(100);
	hm1[228] = "abc";
	CHECK(hash<int>{}(228) % 100 == hm1.bucket(228));
	hm1.insert({ 1337, "d" });
	CHECK(hash<int>{}(1337) % 100 == hm1.bucket(1337));

	CHECK_THROWS(hm1.bucket(123));
}

TEST_CASE("max load factor", "[max_load_factor]") {
	hash_map<int, int> hm1;
	CHECK_THROWS(hm1.max_load_factor(-14));
	CHECK_THROWS(hm1.max_load_factor(14));
	hm1.max_load_factor(0.001);
}

TEST_CASE("equals", "[equals]") {
	hash_map<int, int> hm1 = { {1, 2} };
	hash_map<int, int> hm2(hm1);

	hm2.insert({ 2,3 });

	REQUIRE(!(hm1 == hm2));
	hm1.insert({ 3, 4 });
	REQUIRE(!(hm1 == hm2));
	hm1.insert({ 2,3 });
	hm2.insert({ 3, 4 });
	REQUIRE(hm1 == hm2);
}

TEST_CASE("exceptions for at", "[at]") {
	hash_map<int, int> hm1;
	CHECK_THROWS(hm1.at(1));
	hm1.insert({ 1, 2 });
	CHECK_THROWS(hm1.at(2));

	const hash_map<int, int> hm2{ { 1, 2 } };
	CHECK_THROWS(hm2.at(2));

	const hash_map<int, int> hm3;
	CHECK_THROWS(hm3.at(1));
}

TEST_CASE("reserve", "[reserve]") {
	hash_map<int, string> hm1(6);
	hm1[1] = "test";
	hm1.reserve(45);
	CHECK(hm1.bucket_count() == 100);
	CHECK(hm1[1] == "test");
	hm1[-1] = "test2";
	hm1.reserve(90);
	CHECK(hm1.bucket_count() == 200);
	CHECK(hm1[1] == "test");
	CHECK(hm1[-1] == "test2");
}

TEST_CASE("operator[]", "[operators]") {
	hash_map<int, int> hm1;
	for (int i = 0; i < 100; i++) {
		hm1[i] = i;
	}

	REQUIRE(hm1.size() == 100);
	for (int i = 0; i < 100; i++) {
		REQUIRE(hm1[i] == i);
	}

	hash_map<int, int> hm2;
	hm2[0] = 123;
	hm2[100] = 10000;
	REQUIRE(hm2[0] == 123);
	REQUIRE(hm2[100] == 10000);
	REQUIRE(hm2[4000] == 0);
}

TEST_CASE("get fals ine insert", "[insert]") {
	hash_map<int, string> hm1;
	auto it = hm1.insert(make_pair(0, "abaca"));
	it = hm1.insert(make_pair(0, "cabada"));
	CHECK(hm1.at(0) == "abaca");
	CHECK(!it.second);

	hash_map<int, string> hm2;
	auto pr1 = pair<const int, string>(0, "abaca");
	it = hm2.insert(pr1);
	pr1.second = "cabada";
	it = hm2.insert(pr1);
	CHECK(hm2.at(0) == "abaca");
	CHECK(!it.second);
}

TEST_CASE("get false try_emplace", "[try_emplace]") {
	hash_map<int, string> hm1;
	auto it = hm1.try_emplace(0, "abaca");
	it = hm1.try_emplace(0, "cabada");
	CHECK(hm1.at(0) == "abaca");
	CHECK(!it.second);

	hash_map<int, string> hm2;
	const int a1 = 0;
	it = hm2.try_emplace(a1, "abaca");
	it = hm2.try_emplace(a1, "cabada");
	CHECK(hm2.at(0) == "abaca");
	CHECK(!it.second);
}




TEST_CASE("empty", "[getters]") {
	hash_map<int, int> hm1;
	REQUIRE(hm1.empty());
	hm1.insert({ 1, 2 });
	REQUIRE(!hm1.empty());
}

TEST_CASE("get exceptions in non const iter", "[iterator]") {
	hash_map<int, int> hm1({ {1, 2}, {2, 3}, {3, 4}, {5, 6}, {7, 8} }, 100);
	auto it1 = hm1.begin();
	CHECK_NOTHROW(next(it1, 3));
	CHECK_THROWS(next(it1, 100));
	CHECK_THROWS(fefu::hash_map_iterator<pair<const int, int>>()->first);
	CHECK_THROWS(*fefu::hash_map_iterator<pair<const int, int>>());
}

TEST_CASE("get exceptions in const iter", "[iterator]") {
	hash_map<int, int> hm1{ {1, 2}, {2, 3} };
	auto it1 = fefu::hash_map_const_iterator<pair<const int, int>>(hm1.begin());
	CHECK_NOTHROW(next(it1));
	CHECK_THROWS(next(it1, 3));
	CHECK_THROWS(fefu::hash_map_const_iterator<pair<const int, int>>()->first);
	CHECK_THROWS(*fefu::hash_map_const_iterator<pair<const int, int>>());
}

TEST_CASE("super test allocator", "[allocator]") {
	set<int, less<int>, fefu::allocator<int>> sss;
	for (int i = 0; i < 100; i++) {
		sss.insert(i);
	}

	int idd = 0;
	for (auto iter = sss.begin(); iter != sss.end(); iter++) {
		REQUIRE(*iter == idd++);
	}
}