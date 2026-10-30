#include "dsv.hpp"
#include "check.hpp"
std::string otherTranslationUnit();
int main() { CHECK(otherTranslationUnit().find("header-only") != std::string::npos); }
