#pragma once
#include <string>
#include <vector>

// contains meta-data and actual structure of a pattern/export/import
struct PatternData {
	int width, height;
	int top_left_x = 0, top_left_y = 0;
	std::string name = "";
	std::string attributions = "";

	// Default to standard GoL rules
	//                       0      1      2      3      4      5      6      7      8
	bool birth_rule[9] =    {false, false, false, true , false, false, false, false, false};
	bool survival_rule[9] = {false, false, true , true , false, false, false, false, false};

	std::vector<std::vector<bool>> data_matrix;
	
	// populate this pattern data strucuture based on RLE string, returns false on error.
	bool parse_rle_header(const std::string& line);
	bool parse_rle_data(std::string block);
	bool parse_rle(std::string block);
};
