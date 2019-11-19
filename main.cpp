#include <string>
#include <iostream>
#include <cassert>

#include "hash_map.hpp"

int main() {
  {
    fefu::hash_map<std::string, int> ages(10);

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

	int counter = 0;
    for (auto iter = ages.begin(); iter != ages.end(); iter++) {
      std::cout << iter->first << ": " << iter->second << std::endl;
	  iter->second = counter++;
    }

	ages.insert({ "rvalue refrence", 50 });
	std::pair<std::string, int> some_data = std::pair<std::string, int>("lvalue reference", 12345);
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

    for (auto& vls : data) {
      assert(some_data.at(vls.first) == vls.second);
    }

	for (auto iter = some_data.begin(); iter != some_data.end(); iter++) {
		std::cout << iter->first << ": " << iter->second << std::endl;
	}
  }

  std::cout << "OK" << std::endl;

  return 0;
}
