#include <string>
#include <iostream>
#include <cassert>

#include "hash_map.hpp"

int main() {
  {
    fefu::hash_map<std::string, int> ages(10);
	ages.emplace("Some name", 1);
	ages.try_emplace(std::string("Some name1"), 2);
	std::string some_name3 = "Some name 2";
	ages.try_emplace(some_name3, 3);

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
    } catch (const std::out_of_range& ex) {
    }

    assert(ages["Anya"] == 20);

    assert(ages.contains("Vlad"));
    assert(ages.contains("Kolya"));
    assert(!ages.contains("Kolyas"));

	
	assert(ages.contains("Some name") && ages["Some name"] == 1);
	assert(ages.contains("Some name1") && ages["Some name1"] == 2);
	assert(ages.contains("Some name 2") && ages["Some name 2"] == 3);

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
        {50, 1},     {13466, 25},    {43, 25}, {29, 456}};

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

  std::cout << "OK" << std::endl;

  return 0;
}
