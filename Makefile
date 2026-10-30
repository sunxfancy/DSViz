CXX ?= c++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS += -I.
LDFLAGS ?=
PYTHON ?= python3
DOT ?= dot
BUILD_DIR ?= bin

.PHONY: all basic bst test test-build sanitize release coverage clean FORCE
all: basic bst
basic: $(BUILD_DIR)/basic
bst: $(BUILD_DIR)/bst

# Invalidate binaries when compiler or flags change, even in the same directory.
$(BUILD_DIR)/.flags: FORCE
	@mkdir -p $(BUILD_DIR)
	@$(PYTHON) tests/build_config.py '$@' '$(CXX)' '$(CPPFLAGS)' '$(CXXFLAGS)' '$(LDFLAGS)'

$(BUILD_DIR)/%: example/%.cpp dsv.hpp $(BUILD_DIR)/.flags
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(BUILD_DIR)/regression: tests/regression.cpp tests/check.hpp dsv.hpp $(BUILD_DIR)/.flags
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(BUILD_DIR)/debugger-test: tests/debugger.cpp tests/check.hpp example/bst.cpp dsv.hpp $(BUILD_DIR)/.flags
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(BUILD_DIR)/api-test: tests/api.cpp tests/check.hpp dsv.hpp $(BUILD_DIR)/.flags
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(BUILD_DIR)/header-test: tests/header_a.cpp tests/header_b.cpp tests/check.hpp dsv.hpp $(BUILD_DIR)/.flags
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/header_a.cpp tests/header_b.cpp $(LDFLAGS) -o $@

test-build: all $(BUILD_DIR)/regression $(BUILD_DIR)/debugger-test $(BUILD_DIR)/api-test $(BUILD_DIR)/header-test

test: test-build
	$(PYTHON) tests/run.py --bin-dir $(BUILD_DIR) --dot $(DOT)

sanitize:
	ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 $(MAKE) test BUILD_DIR=bin/sanitize CXXFLAGS='-std=c++11 -Wall -Wextra -Wpedantic -Werror -g -fsanitize=address,undefined -fno-omit-frame-pointer'

release:
	$(MAKE) test BUILD_DIR=bin/release CXXFLAGS='-std=c++11 -Wall -Wextra -Wpedantic -Werror -O2 -DNDEBUG'

coverage:
	$(MAKE) test BUILD_DIR=bin/coverage CXX=g++ CXXFLAGS='-std=c++11 -Wall -Wextra -Wpedantic -Werror -g -O0 --coverage'
	$(PYTHON) -m gcovr --root . --filter 'dsv.hpp' --exclude-unreachable-branches --print-summary --fail-under-line 85 --html-details bin/coverage/coverage.html --xml bin/coverage/coverage.xml bin/coverage

clean:
	rm -rf bin
