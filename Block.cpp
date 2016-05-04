#include "Block.h"

Block::Block() {
	data = new char[blockSize]();
}

Block::~Block() {
	delete data;
}

void Block::overwite(char character, unsigned start, unsigned size) {
	for(unsigned i = start; i < start + size; i++) {
		data[i] = character;
	}
}


std::istream& operator>>(std::istream& input, Block& block) {
    input.read(block.data, block.blockSize);
    return input;
}

std::ostream& operator<<(std::ostream& output, const Block& block) {
    output.write(block.data, block.blockSize);
    return output;
}
