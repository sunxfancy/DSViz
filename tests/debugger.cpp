#define main example_main
#include "../example/bst.cpp"
#undef main
#include "check.hpp"
int main() {
    bst tree;
    for (int i = 0; i < 150; ++i) tree.insert(i);
    std::string result = _dotToDebugger(tree);
    CHECK(result.size() > 16384);
    CHECK(result.find("149") != std::string::npos);
    CHECK(_dotToDebuggerSize() == result.size());
    bst empty; CHECK(std::string(_dotToDebugger(empty)).empty());
    CHECK(_dotToDebuggerSize() == 0);
}
