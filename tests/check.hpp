#pragma once
#include <stdexcept>
#include <string>
// Unlike assert(), checks remain active with -DNDEBUG in release builds.
#define CHECK(expression) do { if (!(expression)) throw std::runtime_error( \
    std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #expression); } while (false)
template<class Exception, class F> void checkThrows(F f) {
    bool caught = false;
    try { f(); } catch (const Exception &) { caught = true; }
    CHECK(caught);
}
