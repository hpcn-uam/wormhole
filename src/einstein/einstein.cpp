#include "einstein.hpp"

EinsConn::~EinsConn() {
    this->deleteAllWorms();
}