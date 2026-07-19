#include "grid.hpp"
#include <cstring>


// Helper function to create a hashing key by combining two ints into a single uint64
uint64_t create_coord_hashkey(int x, int y) {
	uint64_t left_32_bits = (uint32_t)x;
	uint64_t right_32_bits = (uint32_t)y;
	return ((left_32_bits << 32) | right_32_bits);
}

bool Chunk::nei_exists(ChunkDir chunk_direction) {
	return (neighbours[chunk_direction] != nullptr);
}

// helper function to get a row as usual 
// but return row from above or below chunk if out of bounds of this chunk's data
uint64_t* Chunk::get_row(int i) {
	if (i > 63) return nei_exists(CHUNKDIR_NORTH) ? &(neighbours[CHUNKDIR_NORTH]->data[0]) : nullptr;
	if (i < 0) return nei_exists(CHUNKDIR_SOUTH) ? &(neighbours[CHUNKDIR_SOUTH]->data[63]) : nullptr;
	return &(data[i]);
}

// helper function to increment count_bits based on a realigned neighbour row
void update_count_bits(uint64_t *count_bits, uint64_t row) {
	// row acts as initial carry
	uint64_t carry = row;
 	for (int i = 0; i < 4; i++) {
		// calculate carry before reassigning count_bit[i]
		uint64_t next_carry = count_bits[i] & carry;
		// sum bits using XOR w/o carry
		count_bits[i] = count_bits[i] ^ carry;
		// update carry for the next iteration
		carry = next_carry;
	}
}

// helper function to get edge bits of a row from EAST, WEST, NORTH EAST, and NORTH WEST chunks
// to access NE and NW bits pass row 64/-1 of E or W. do NOT pass NE or NW.
uint64_t Chunk::get_edge_bit(ChunkDir target_neighbour, int row) {
	if (!nei_exists(target_neighbour)) return 0;
	// get thte row.
	auto* extracted_row_ptr = neighbours[target_neighbour]->get_row(row);
	uint64_t extracted_row = (extracted_row_ptr != nullptr) ? *extracted_row_ptr : 0;
	if (target_neighbour == CHUNKDIR_EAST) {
		uint64_t target_bit = (uint64_t)1;
		return (extracted_row & target_bit) << 63;
	} else { // west
		uint64_t target_bit = (uint64_t)1 << 63;
		return (extracted_row & target_bit) >> 63;
	}
}

// Note:
// increasing 'data' row index represents moving NORTH
// the right bits on the uint64's represent EAST
void Chunk::process() {
	// Loop through every row (from south to north)
	for (int i = 0; i < 64; i++) {
		// create new copies of data rows.
		// so that all the neighbour bits align onto this row 
		// and can be summed to find total neighbour of each cell at once
		// >> go up/down a row to get top/bottom neighbours aligned
		auto* n_ptr = get_row(i+1); uint64_t n = (n_ptr != nullptr)? *n_ptr : 0;
		auto* s_ptr = get_row(i-1); uint64_t s = (s_ptr != nullptr)? *s_ptr : 0;
		// >> shift current row by a bit to align neighbours to the left/right
		uint64_t e = data[i] >> 1;
		uint64_t w = data[i] << 1;
		// >> now combine up/down and left/right to get diagonal neighbours
		uint64_t ne = n >> 1;
		uint64_t nw = n << 1;
		uint64_t se = s >> 1;
		uint64_t sw = s << 1;
		// >> get edge bits from neighbouring chunks
		uint64_t bit_e = get_edge_bit(CHUNKDIR_EAST, i);
		uint64_t bit_w = get_edge_bit(CHUNKDIR_WEST, i);
		// >> we also need neighbour diagonal neighbour bits
		uint64_t bit_ne = get_edge_bit(CHUNKDIR_EAST, i+1);
		uint64_t bit_nw = get_edge_bit(CHUNKDIR_WEST, i+1);
		uint64_t bit_se = get_edge_bit(CHUNKDIR_EAST, i-1);
		uint64_t bit_sw = get_edge_bit(CHUNKDIR_WEST, i-1);
		// >> glue neighbour bits onto shifted rows (e,w, ne, nw, se, sw)
		e |= bit_e; w |= bit_w;
		ne |= bit_ne; nw |= bit_nw;
		se |= bit_se; sw |= bit_sw;
		
		// Now we need to count amt of neighbours by using
		// Binary counters of the same amt of bits as data so that each bit maps to a cell
		// Each array stores a bit with a higher place value, this allowed us count until 2^3
		// Which is enough to count the neighbour cells
		uint64_t count_bits[4] = {0};
		update_count_bits(count_bits, n); update_count_bits(count_bits, s);
		update_count_bits(count_bits, e); update_count_bits(count_bits, w);
		update_count_bits(count_bits, ne); update_count_bits(count_bits, nw);
		update_count_bits(count_bits, se); update_count_bits(count_bits, sw);

		// Apply Game Of Life rules 
		// two =                           0                0                1               0
		uint64_t cells_with_2_neighbours = ~count_bits[3] & ~count_bits[2] & count_bits[1] & ~count_bits[0];
		// three =                         0                0                1               1
		uint64_t cells_with_3_neighbours = ~count_bits[3] & ~count_bits[2] & count_bits[1] & count_bits[0];
		// All cells with 3 neighbours are alive in the next generation (either dead cell is reborn or alive cell stays alive)
		// All alive cells with 2 neighbours are alive in the next gen (not overpopulated or underpopulated)
		new_data[i] = cells_with_3_neighbours | (data[i] & cells_with_2_neighbours);
	}
}

void Chunk::update() {
	std::memcpy(data, new_data, sizeof(data));
}

// -- 
// -- SPARSE CHUNK GRID
// -- 

Chunk* SparseChunkGrid::get_chunk(int x, int y) {
	uint64_t key = create_coord_hashkey(x, y);
	auto iterator = chunks.find(key);
	if (iterator != chunks.end()) {
		return &(iterator->second);
	}
	return nullptr;
}

void SparseChunkGrid::interlink_chunk(int x, int y) {
	Chunk* chunk = get_chunk(x, y);
	// Relative coordinate mappings to get to each ChunkDir
	int offset_x[] = {0, 0, 1, -1, 1, -1, 1, -1};
	int offset_y[] = {1, -1, 0, 0, 1, 1, -1, -1};
	ChunkDir opposite[] = {
		CHUNKDIR_SOUTH, CHUNKDIR_NORTH, CHUNKDIR_WEST, CHUNKDIR_EAST,
		CHUNKDIR_S_WEST, CHUNKDIR_S_EAST, CHUNKDIR_N_WEST, CHUNKDIR_N_EAST,
	};
	for(int i = 0; i < 8; i++) {
		Chunk* neighbour = get_chunk(x+offset_x[i], y+offset_y[i]);
		if (neighbour != nullptr) {
			// Link BOTH the current chunk and the neighbour
			chunk->neighbours[i] = neighbour;
			neighbour->neighbours[opposite[i]] = chunk;
		}
	}
}

void SparseChunkGrid::place_chunk(int x, int y) {
	uint64_t key = create_coord_hashkey(x, y);
	if (chunks.find(key) == chunks.end()) {
		chunks.emplace(key, Chunk(x, y));
		interlink_chunk(x, y);
	}
};

void SparseChunkGrid::toggle_cell(int cell_x, int cell_y) {
	// Which chunk do these coordinates fall under?
	int chunk_x = world_to_chunk_x(cell_x);
	int chunk_y = world_to_chunk_y(cell_y);
	// Where in the chunk to they fall?
	int inner_x = world_to_local_x(cell_x);
	int inner_y = world_to_local_y(cell_y);
	// place chunk if it doesn't already exist
	place_chunk(chunk_x, chunk_y);
	// get the chunk
	auto &chunk = chunks.find(create_coord_hashkey(chunk_x, chunk_y))->second;
	uint64_t mask = (uint64_t)1 << inner_x;
	chunk.data[inner_y] ^= mask;
}

void SparseChunkGrid::step() {
	// Figure out where new chunks need to be created
	// By checking if there are cells on the edges
	std::vector<std::pair<int, int>> pending_coords;
	for (auto& key_and_chunk : chunks) {
		auto &chunk = key_and_chunk.second;
		// North edge
		if (chunk.data[63] != 0 && !chunk.nei_exists(CHUNKDIR_NORTH)) {
			pending_coords.push_back({chunk.macro_x, chunk.macro_y+1});
		}
		// South edge
		if (chunk.data[0] != 0 && !chunk.nei_exists(CHUNKDIR_SOUTH)) {
			pending_coords.push_back({chunk.macro_x, chunk.macro_y-1});
		}
		// The east and west edges can be found by just OR'ing all the rows
		// And then seeing if there is a 1 in the first or last column
		uint64_t squashed_rows = 0;
		for (int r = 0; r < 64; r++) squashed_rows |= chunk.data[r];
		// East
		if ((squashed_rows & (uint64_t)1 << 63) != 0 && !chunk.nei_exists(CHUNKDIR_EAST)) {
			pending_coords.push_back({chunk.macro_x+1, chunk.macro_y});
		}
		// West
		if ((squashed_rows & ((uint64_t)1)) != 0 && !chunk.nei_exists(CHUNKDIR_WEST)) {
			pending_coords.push_back({chunk.macro_x-1, chunk.macro_y});
		}
		// Diagonals
        // North-East
        if ((chunk.data[63] & (uint64_t)1) != 0 && !chunk.nei_exists(CHUNKDIR_N_EAST))
            pending_coords.push_back({chunk.macro_x + 1, chunk.macro_y + 1});
        // North-West
        if ((chunk.data[63] & ((uint64_t)1 << 63)) != 0 && !chunk.nei_exists(CHUNKDIR_N_WEST))
            pending_coords.push_back({chunk.macro_x - 1, chunk.macro_y + 1});
        // South-East
        if ((chunk.data[0] & (uint64_t)1) != 0 && !chunk.nei_exists(CHUNKDIR_S_EAST))
            pending_coords.push_back({chunk.macro_x + 1, chunk.macro_y - 1});
        // South-West
        if ((chunk.data[0] & ((uint64_t)1 << 63)) != 0 && !chunk.nei_exists(CHUNKDIR_S_WEST))
            pending_coords.push_back({chunk.macro_x - 1, chunk.macro_y - 1});
	}
	// Create and link the chunks
	for (auto& coords : pending_coords) {
		place_chunk(coords.first, coords.second);
	}

	// Process all chunks
	for (auto &key_and_chunk : chunks) key_and_chunk.second.process();
	// Update all chunks
	for (auto &key_and_chunk : chunks) key_and_chunk.second.update();
}

