#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>

// todo
// Hash Function
// Hash Loading
// Hash Lookup
// Dynamically resizing hash table once insufficient

// helper functinos to convert between world coordinates and macro/chunk coordinates as well as inner-chunk coordinates
inline int world_to_chunk_x(int world_x) { return world_x >> 6; }
inline int world_to_chunk_y(int world_y) { return world_y >> 6; }
inline int world_to_local_x(int world_x) { return world_x & 63; }
inline int world_to_local_y(int world_y) { return world_y & 63; }

// Chunks are a sqaure region of space containing cells, generated when an alive cell enters any neighbour chunks.
// neighbour chunk direction enum
enum ChunkDir {
	CHUNKDIR_NORTH, CHUNKDIR_SOUTH, CHUNKDIR_EAST, CHUNKDIR_WEST,
	CHUNKDIR_N_EAST, CHUNKDIR_N_WEST, CHUNKDIR_S_EAST, CHUNKDIR_S_WEST,
	CHUNKDIR_SIZE,
};
struct Chunk {
	int macro_x, macro_y;
	Chunk *neighbours[CHUNKDIR_SIZE] = {nullptr}; 
	// Each uint64 represents one row and each bit inside represents one column
	uint64_t data[64] = {0};
	// Temporary storage to write to during processing loop
	uint64_t new_data[64] = {0};
	Chunk(int pos_x, int pos_y) : macro_x(pos_x), macro_y(pos_y) {}

	// Helper function to check if a certain neighbour exists
	bool nei_exists(ChunkDir chunk_direction);
	uint64_t get_edge_bit(ChunkDir target_neighbour, int row);
	uint64_t* get_row(int i);

	void process();
	// This function updates data with new_data.
	// this MUST be called only after all chunks have finished processing!
	void update();
};

struct SparseChunkGrid {
	// Active chunks
	std::unordered_map<uint64_t, Chunk> chunks;
	// Helper function to lookup chunk coords, if it doesn't exist, returns nullptr
	Chunk* get_chunk(int x, int y);
	// Helper function to automatically link chunks together as neighbours
	void interlink_chunk(int x, int y);
	// Place and link a Chunk on the grid
	void place_chunk(int x, int y);
	// Toggle a cell, pass coordinates on a cell-level, it'll automatically create the chunk if it doesn't already exist
	void toggle_cell(int cell_x, int cell_y);
	// Create, process, and update all chunks
	void step();
};
