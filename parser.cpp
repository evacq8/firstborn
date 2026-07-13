#include <vector>
#include <string>
#include <iostream>

#include "parser.hpp"

std::vector<std::string> break_str_into_lines(std::string block) {
	std::vector<std::string> lines;
	int last_break = 0;
	for (int c = 0; c <= block.length(); c++) {
		// run until you find a line break, then extract substring, mark it down in last_break and keep going
		if (block[c] == '\n' || c == block.length()) {
			lines.push_back(block.substr(last_break, c-last_break));
			last_break = c+1;
		}
	}
	return lines;
}

bool PatternData::read_rle(std::string content) {
	auto lines = break_str_into_lines(content);

	// run length tracking
	int loc_x = 0;
	int loc_y = 0;
	// string to store the current number, resets upon hitting a symbol
	std::string running_digits = "";

	for (auto line : lines) {
		// detect # meta data lines
		if (line[0] == '#') {
			if (line[1] == 'N') {
				name = line.substr(3, line.length());
			} else if (line[1] == 'O') {
				attributions = line.substr(3, line.length());
			} else if (line[1] == 'R') {
				// e.g. #R -22 -12
				// find second space
				int second_space_pos;
				for (int c = 3; c < line.length(); c++) if (line[c] == ' ') second_space_pos = c;
				// capture the first number
				top_left_x = std::stoi(line.substr(3, second_space_pos-3));
				// capture the second number
				top_left_y = std::stoi(line.substr(second_space_pos+1, line.length()-second_space_pos+1));
			}
		}
		// detect header line
		else if (line[0] == 'x') {
			// e.g. x = 36, y = 9
			// find first equal sign
			int eq_1;
			for (int c = 1; c < line.length(); c++) if (line[c] == '=') { eq_1 = c; break; }
			// find next comma
			int comma_1;
			for (int c = eq_1; c < line.length(); c++) if (line[c] == ',') { comma_1 = c; break; }
			// find next equal sign
			int eq_2;
			for (int c = comma_1; c < line.length(); c++) if (line[c] == '=') { eq_2 = c; break; }
			// find end of line or comma or space
			int comma_2 = line.length();
			for (int c = eq_2+2; c < line.length(); c++) if (line[c] == ',') { comma_2 = c; break; }
			// extract x value
			size_x = std::stoi(line.substr(eq_1+1, comma_1-1-eq_1));
			size_y = std::stoi(line.substr(eq_2+1, comma_2-1-eq_2));

			// initialize data in accordance to inputted dimensions
			data_matrix = std::vector<std::vector<bool>>(size_y, std::vector<bool>(size_x, false));
		}
		// if none of the above, treat it as RLE data
		else {
			for (auto chara : line) {
				if (chara == 'b' || chara == 'o' || chara == 'g' || chara == 'x' || chara == 'y' || chara == 'z') {
					int run_len = (running_digits.length() != 0) ? std::stoi(running_digits) : 1;
					while (run_len > 0) {
						data_matrix[loc_y][loc_x] = (chara == 'b') ? false : true;
						loc_x++;
						run_len--;
					}
					running_digits = "";
				} else if (chara == '$') {
					loc_y++; 
					loc_x = 0;
				}
				else if (chara == '!') break;
				else { // digits
					running_digits += chara;
				}
			}
		}
	}
	return true;
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
