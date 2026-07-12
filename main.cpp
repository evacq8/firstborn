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

#define GRID_WIDTH 2048
#define GRID_HEIGHT 2048



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
void move_cursor_up(unsigned int n) {
	std::cout << "\x1B[" << n << "A" << std::flush;
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
	int center_x = (GRID_WIDTH/2);
	int center_y = (GRID_HEIGHT/2);
	int height, width;
	void render(const CellMatrix& grid) {
		int start_y = center_y - (height/2);
		int end_y = center_y + (height/2);
		int start_x = center_x - (width/4);
		int end_x = center_x + (width/4);
		for (int y = start_y; y < end_y-1; y++) {
			for (int x = start_x; x < end_x; x++) {
				if (grid[y][x] == true)
					std::cout << "██";
				else
					std::cout << "  ";
			}
			std::cout << "\n";
		}
	}
};

int count_amt_neighbours(const CellMatrix& grid, int x, int y) {
	int count = 0;
	for (int offset_x : {-1, 0, 1}) {
		for (int offset_y : {-1, 0, 1}) {
			if(offset_x == 0 && offset_y == 0) continue;
			// skip if out of bounds (considered dead)
			if (x+offset_x < 0 || x+offset_x >= GRID_WIDTH || y+offset_y < 0 || y+offset_y >= GRID_HEIGHT) continue;
			if (grid[y+offset_y][x+offset_x] == true) count++;
		}
	}
	return count;
}

// determine a theoretical cell's next state based on its current state and amt. of neighbours
bool determine_cell_state(bool state, int amt_neighbours) {
	if (state == true) {
		// underpopulation
		if (amt_neighbours < 2) return false;
		// overpopulation
		if (amt_neighbours > 3) return false;
	} else {
		// reproduction
		if (amt_neighbours == 3) return true;
	}
	return state;
}

void simulation_step(CellMatrix& grid) {
	CellMatrix write_buf(GRID_HEIGHT, std::vector<Cell>(GRID_WIDTH));
	for (int y = 0; y < GRID_HEIGHT; y++) {
		for (int x = 0; x < GRID_WIDTH; x++) {
			int neighbour_count = count_amt_neighbours(grid, x, y);
			write_buf[y][x] = determine_cell_state(grid[y][x], neighbour_count);
		}
	}
	// apply changes
	grid = write_buf;
}

int main() {
	CellMatrix grid(GRID_HEIGHT, std::vector<Cell>(GRID_WIDTH, false));
	Camera cam;
	get_terminal_dimensions(cam.height, cam.width);

	const int cx = 1024;
    const int cy = 1024;
    grid[cy - 10][cx - 10] = true;
    grid[cy - 9][cx - 9] = true;
    grid[cy - 8][cx - 9] = true;
    grid[cy - 8][cx - 10] = true;
    grid[cy - 8][cx - 11] = true;

	float fps = 10;
	std::chrono::duration<float> frame_duration(1.0f/fps);
	int frame = 0;
	while (true) {
		frame++;
		if (frame % 5 == 0) simulation_step(grid);
	
		std::this_thread::sleep_for(frame_duration);
		char input_char = get_latest_char_input();
		move_cursor_up(cam.height);
		cam.render(grid);
	}
}

