#pragma once
#include <string>
#include <vector>

// contains meta-data and actual structure of a pattern/export/import
struct PatternData {
	int size_x, size_y;
	std::string name = "unnamed pattern";
	std::string attributions = "no attribution";
	int top_left_x = 0, top_left_y = 0;
	std::vector<std::vector<bool>> data_matrix;
	// populate this pattern data strucuture based on RLE string, returns false on error.
	bool read_rle(std::string content);
};
