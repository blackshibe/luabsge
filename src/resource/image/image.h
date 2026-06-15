#pragma once

#include <string>
#include <vector>

struct ImageData {
	std::string name;

	int width = 0;
	int height = 0;
	std::vector<unsigned char> pixels;

  public:
	ImageData(std::string name) : name(name) {}
};
