#include <vector>
#include <string>
#include <iostream>
#include <regex>

#include "parser.hpp"

std::vector<std::string> break_str_into_lines(std::string block) {
	std::vector<std::string> lines;
	int last_break = 0;
	for (int c = 0; c <= block.length(); c++) {
		// run until you find a line break, then extract substring, mark it down in last_break and keep going
		if (c == block.length() || block[c] == '\n') {
			lines.push_back(block.substr(last_break, c-last_break));
			last_break = c+1;
		}
	}
	return lines;
}

bool PatternData::parse_rle_header(const std::string& line) {
	//                         x   =   ( 1 )   ,   y   =   ( 2 )(?    ,   rule   =   B( 3 )/S( 4 )?)
	std::regex header_regex(R"(x\s*=\s*(\d+)\s*,\s*y\s*=\s*(\d+)(?:\s*,\s*rule\s*=\s*B(\d*)/S(\d*))?)");
	std::smatch matches;
	if (std::regex_search(line, matches, header_regex)) {
		width = std::stoi(matches[1].str());
		height = std::stoi(matches[2].str());
		top_left_x = -width/2;
		top_left_y = width/2;
		// check if 3 and 4 exist cuz' those were optional
		if (matches[3].matched && matches[4].matched) {
			// Reset both rules
			for (int i = 0; i < 9; i++) { 
				birth_rule[i] = false;
				survival_rule[i] = false;
			}

			std::string birth_str = matches[3].str();
			std::string survival_str = matches[4].str();
			// Loop through each digit in each rule string
			for (char chara : birth_str) {
				int num = chara - '0';
				if(num > 8 || num < 0) { std::cout << "Unsupported digit in birth rule."; return false; }
				birth_rule[num] = true;
			}
			for (char chara : survival_str) {
				int num = chara - '0';
				if(num > 8 || num < 0) { std::cout << "Unsupported digit in survival rule."; return false; }
				survival_rule[num] = true;
			}
		}
	} else {
		std::cout << "Couldn't Parse RLE Header...\n";
		return false;
	}
	return true;
}

bool PatternData::parse_rle_data(std::string block) {
	// Resize data matrix
	data_matrix = std::vector<std::vector<bool>>(height, std::vector<bool>(width, false));

	std::regex token_reg(R"((\d*)([bo$\!]))");
	auto tokens_begin = std::sregex_iterator(block.begin(), block.end(), token_reg);
	auto tokens_end = std::sregex_iterator();
	int current_x = 0;
	int current_y = 0;
	// Iterate throgugh pairs of (COUNT:SYMBOL)
	for (std::sregex_iterator i = tokens_begin; i != tokens_end; ++i) {
		std::smatch match = *i;
		auto count_str = match[1].str();
		int count = count_str.empty() ? 1 : std::stoi(count_str);

		char symbol = match[2].str()[0];
		if(symbol == '!') break; // File end
		else if(symbol == '$') { // Skip line(s)
			current_y += count;
			current_x = 0;
		} else if(symbol == 'o') { // Place live cell(s)
			while (count > 0) {
				data_matrix[(height-1)-current_y][current_x] = true;
				current_x++;
				count--;
			}
		} else if(symbol == 'b') { // Place dead cell(s)
			while (count > 0) {
				data_matrix[(height-1)-current_y][current_x] = false;
				current_x++;
				count--;
			}
		} else {
			std::cout << "Unrecognized symbol '" << symbol << "'\n";
			return false;
		}
	}
	return true;
}

bool PatternData::parse_rle(std::string block) {
	std::vector<std::string> lines = break_str_into_lines(block);
	std::string rle_data = "";
	for (std::string line : lines) {
		if (line.empty()) continue;
		// META / COMMENT LINES
		if (line[0] == '#') {
			// Name
			if      (line[1] == 'N') name = line.substr(3, line.length());
			// Attributions
			else if (line[1] == 'O') attributions = line.substr(3, line.length());
			// Top-left coordinates
			else if (line[1] == 'R') {
				std::regex reg(R"(#R\s*(-?\d+)\s*(-?\d+))");
				std::smatch matches;
				if (std::regex_search(line, matches, reg)) {
					top_left_x = std::stoi(matches[1].str());
					top_left_y = std::stoi(matches[2].str());
				}
			}
		} else if (line[0] == 'x') {
			if (!parse_rle_header(line)) return false;
		} else {
			rle_data += line;
		}
	}
	return parse_rle_data(rle_data);
}

/*
int main() {
	std::string str = "#N Gosper Glider Gun\n#O Bill Gosper\n#R -22 -12\nx = 36, y = 9, rule = B3/S23\n24bo$22bobo$12b2g6b2g12b2g$11bo3bo4b2g12b2g$2b2g5bo5bo3b2g$2b2g5bo3bo\n4b2g$11bo5bo$12b3g$14bo!";
	PatternData pattern;
	pattern.read_rle(str);
	std::cout << "Pattern name: " << pattern.name << "\n";
	std::cout << "Attributions: " << pattern.attributions << "\n";
	std::cout << "Top left coords: " << pattern.top_left_x << " " << pattern.top_left_y << "\n";
	std::cout << "Dimensions: " << pattern.size_x << " " << pattern.size_y << "\n";
	for (auto vec : pattern.data_matrix) {
		for (auto cell : vec) {
			std::cout << ((cell) ? "#" : ".");
		}
		std::cout << "\n";
	}
}
*/
