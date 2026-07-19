#if defined(__unix__) || defined(__APPLE__) && defined(__MACH__)
	#define IS_POSIX 1
#else
	#define IS_POSIX 0
#endif

#if IS_POSIX
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include "parser.hpp"
#include "grid.hpp"

// https://stackoverflow.com/a/1022961
void get_terminal_dimensions(int& rows, int& cols)
{
#if IS_POSIX
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	rows = w.ws_row;
	cols = w.ws_col;
#else
	std::cout << "TODO: Windows support\n";
#endif
}

// https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b
void move_cursor_top() {
	std::cout << "\x1B[1;1H" << std::flush;
}

// https://stackoverflow.com/a/912796
// get non-blocking input, returns \0 if no character
char get_latest_char_input() {
	char chara = 0; // not an undertale reference

	struct termios old = {0};
	tcgetattr(0, &old);
	// disable line-buffering (pressing enter) and echo for the terminal
	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 0; // don't wait for a character
	old.c_cc[VTIME] = 0;
	// apply and read input
	tcsetattr(0, TCSANOW, &old);

	// read only the latest character in the buffer (iterate/discard everything else)
	while (read(0, &chara, 1) > 0);
	
	// revert original settings
	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;
	tcsetattr(0, TCSADRAIN, &old);

	return chara;
}

using CellMatrix = std::vector<std::vector<bool>>;
using Cell = bool;

// stores info & methods relating to the view
struct Camera {
	int center_x = 0;
	int center_y = 0;
	int height, width;
	void render(SparseChunkGrid &grid, bool paused);
};

void Camera::render(SparseChunkGrid &grid, bool paused) {

	std::string frame_output = "\x1B[1;1H";
	int start_y = center_y - (height/2);
	int end_y = center_y + (height/2);
	int start_x = center_x - (width/4);
	int end_x = center_x + (width/4);
	// Loop through every coordinate visible by the camera
	for (int y = end_y-1; y >= start_y; y--) {
		for (int x = start_x; x < end_x; x++) {
			// Get the current chunk
			int px_chunk_x = world_to_chunk_x(x);
			int px_chunk_y = world_to_chunk_y(y);
			Chunk* px_chunk = grid.get_chunk(px_chunk_x, px_chunk_y);
			if (px_chunk == nullptr) {
				frame_output += "░░";
			} else {
				// Calculate where in the chunk we are
				int inner_x = world_to_local_x(x);
				int inner_y = world_to_local_y(y);
				uint64_t bit_extract_mask = (uint64_t)1 << inner_x;
				bool is_alive = (px_chunk->data[inner_y] & bit_extract_mask) != 0;
				if (x == center_x && y == center_y) {
					frame_output += is_alive ? "▓▓" : "░░";
				} else {
					frame_output += is_alive ? "██" : "  ";
				}
			}
			
		}
		if (y != start_y) { // Line breaks
			frame_output += "\n";
		} else { // Custom last line
			frame_output += paused ? "[SIMULATION PAUSED]" : "[SIMULATION RUNNING]";
			frame_output += " Cursor: (" + std::to_string(center_x) + ", " + std::to_string(center_y);
			frame_output += ") Chunk: (" + std::to_string(world_to_chunk_x(center_x)) + ", " + std::to_string(world_to_chunk_y(center_y));
			frame_output += ") ... ";
		}
	}
	std::cout << frame_output << std::flush;
}

int main(int argc, char* argv[]) {
	SparseChunkGrid grid;

	Camera cam;
	get_terminal_dimensions(cam.height, cam.width);

	std::cout << "\x1B[?25l" << std::flush;

	// Load a pattern 
	if(argc > 1) {
		std::ifstream rle_file(argv[1]);
		if(!rle_file.is_open()) {
			std::cout << "Couldn't find file\n";
			return 1;
		}
		std::string line;
		std::string text;
		while (std::getline(rle_file, line)) text += line + "\n";
		PatternData pattern;
		pattern.read_rle(text);

		for (int y = 0; y < pattern.size_y; y++) {
			for (int x = 0; x < pattern.size_x; x++) {
				int target_y = pattern.top_left_y + y;
				int target_x = pattern.top_left_x + x;
				if (pattern.data_matrix[y][x] == true) grid.toggle_cell(target_x, target_y);
			}
		}
	} 

	float fps = 30;
	std::chrono::duration<float> frame_duration(1.0f/fps);
	int frame = 0;
	bool paused = true;

	while (true) {
		frame++;

		if (frame % 3 == 0 && !paused) grid.step();
		get_terminal_dimensions(cam.height, cam.width);


		std::this_thread::sleep_for(frame_duration);
		char input_char = get_latest_char_input();
		
		// input stuff
		if (input_char == 'h' || input_char == 'a')
			cam.center_x--;
		else if(input_char == 'j' || input_char == 's')
			cam.center_y--;
		else if(input_char == 'k' || input_char == 'w')
			cam.center_y++;
		else if(input_char == 'l' || input_char == 'd')
			cam.center_x++;
		else if(input_char == ' ') {
			grid.toggle_cell(cam.center_x, cam.center_y);
		}
		else if(input_char == '\n')
			paused = paused ? false : true;
		else if(input_char == 'q')
			break;

		cam.render(grid, paused);
	}
}

