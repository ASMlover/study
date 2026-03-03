#include "ms/chunk.hh"

#include <stdexcept>

namespace ms {

void Chunk::writeChunk(uint8_t byte, int line) {
    code_.push_back(byte);
    lines_.push_back(line);
}

int Chunk::addConstant(Value value) {
    constants_.push_back(value);
    return static_cast<int>(constants_.size()) - 1;
}

Value Chunk::getConstant(int index) const {
    if (index < 0 || index >= static_cast<int>(constants_.size())) {
        throw std::out_of_range("Constant index out of range");
    }
    return constants_[index];
}

uint8_t Chunk::getCode(int index) const {
    if (index < 0 || index >= static_cast<int>(code_.size())) {
        throw std::out_of_range("Code index out of range");
    }
    return code_[index];
}

int Chunk::getLine(int index) const {
    if (index < 0 || index >= static_cast<int>(lines_.size())) {
        throw std::out_of_range("Line index out of range");
    }
    return lines_[index];
}

size_t Chunk::count() const {
    return code_.size();
}

void Chunk::clear() {
    code_.clear();
    constants_.clear();
    lines_.clear();
}

}
