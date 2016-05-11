#include "Block.h"

Block::Block() {
	data = new char[blockSize]();
}

Block::Block(Block& b) {
	data = new char[blockSize]();
	for (unsigned i = 0; i < blockSize; i++) {
		data[i] = b.data[i];
	}
}

Block::~Block() {
	delete[] data;
}

void Block::overwrite(char character, unsigned start, unsigned size) {
	for (unsigned i = start; i < start + size; i++) {
		data[i] = character;
	}
}

void Block::overwrite(void* characters, unsigned start, unsigned size) {
	memcpy(&(data[start]), characters, size);
}

std::string Block::getStringOfLength(unsigned lengthBytes) {
	return std::string(data, lengthBytes);
}

unsigned Block::readUnsignedAtIndex(unsigned index) {
	unsigned result = 0;
	memcpy(&result, &(data[index]), sizeof(unsigned));
	return result;
}


Block& Block::operator=(const Block& other) {
    if (&other == this) {
        return *this;
    }

	delete[] data;
	data = new char[blockSize]();
	for (unsigned i = 0; i < blockSize; i++) {
		data[i] = other.data[i];
	}
	return *this;
}

std::istream& operator>>(std::istream& input, Block& block) {
    input.read(block.data, block.blockSize);
    return input;
}

std::ostream& operator<<(std::ostream& output, const Block& block) {
    output.write(block.data, block.blockSize);
    return output;
}
