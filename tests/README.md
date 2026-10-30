# Tests and CI

## Requirements and commands

Ordinary tests need GNU Make, a C++11 compiler, Python 3.8+, and GraphViz (`dot`).
They require no downloaded C++ test framework and no running debugger.

```sh
make -j2 test       # Debug, strict warnings, API and integration tests
make -j2 release    # Optimized build with NDEBUG; checks remain enabled
make -j2 sanitize   # AddressSanitizer, UndefinedBehaviorSanitizer, leak detection
make -j2 test CXX=clang++ BUILD_DIR=bin/clang
make -j2 test CXXFLAGS='-std=c++20 -Wall -Wextra -Wpedantic -Werror -g' BUILD_DIR=bin/cxx20
```

Override `PYTHON`, `DOT`, `CXX`, `CPPFLAGS`, `CXXFLAGS`, `LDFLAGS`, and
`BUILD_DIR` as needed. Compiler/flag changes invalidate binaries through a build
stamp. Use different build directories for concurrent configurations.

For coverage, install the optional pinned reporter in a virtual environment:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r tests/requirements.txt
make -j2 coverage PYTHON=.venv/bin/python
```

Coverage uses GCC/gcov and measures `dsv.hpp`, excluding test and example code.
The build fails below **85% line coverage**. Branch coverage is reported but
not gated: compiler-generated exception branches are included. HTML and
Cobertura XML are written under `bin/coverage/`. A system `gcov` matching the
selected `g++` is required. Sanitizers are intended for Linux GCC/Clang.

## Test layers

| Layer | Source | Checks |
| --- | --- | --- |
| API regressions | `regression.cpp` | Null roots, cycles, mapping consistency, escaping, parallel edges, explicit submission, reset and streaming |
| API scenarios | `api.cpp` | Invalid inputs, scalar/string/bool fields, arrays, children, shared nodes, direct C-style adapters, raw attributes, nested subgraphs, configuration, 1000-node output |
| Header-only linkage | `header_a.cpp`, `header_b.cpp` | Independent includes and linking specializations from multiple translation units |
| Debugger buffer | `debugger.cpp` | More than 16 KiB output, exact size, empty-tree reset; sanitizer checks ownership |
| Python bridge | `debugger_test.py` | UTF-8, HTML escaping, expression failures, null pointers, size limit, read failures and incomplete reads |
| GraphViz integration | `run.py` | Generated DOT actually parses to SVG rather than only matching strings |

`check.hpp` deliberately avoids the standard `assert` macro, so Release tests
still fail on incorrect results. Each scenario runs in a separate subprocess
with a 60-second timeout. Failures do not stop later scenarios. Any failure,
missing dependency, crash, timeout, or invalid DOT gives a nonzero exit status.
Logs, DOT, SVG, and `junit.xml` are retained in `<BUILD_DIR>/test-results/`.
The Python unit suite reports its individual tests in the captured log and is
one aggregate case in JUnit.

## GitHub Actions

The `Tests` workflow runs on pushes, pull requests, and manual dispatch:

- Linux GCC and Clang × C++11/17/20 × Debug/Release.
- macOS Apple Clang, C++11 Release.
- Linux GCC and Clang ASan/UBSan with leak detection enabled.
- GCC coverage with an 85% line gate.

Jobs have timeouts, read-only repository permissions, and cancel obsolete runs
for the same ref. Reports are uploaded even after failures and retained for
14 days. The final **CI required** job fails if any dependency fails or is
cancelled; select it as a required check in GitHub branch protection if desired.
Branch protection itself is not configured by this repository.

To add a C++ API scenario, add its function and dispatch entry in `api.cpp`, then
register it in `run.py`. Emit DOT to stdout so GraphViz validates it; diagnostics
go to stderr. Add Python cases to `TransportTests` using the fake LLDB session.
Avoid external network access, wall-clock assumptions, and exact SVG snapshots
(fonts/layout vary across GraphViz versions).

## Scope

These tests validate the supported C++ APIs and generated graphs. The fake LLDB
session does not replace a manual CodeLLDB UI smoke test. Windows/MSVC, arbitrary
depth traversal, concurrent access to the Mock cache, and untrusted raw DOT
attributes are not claimed as supported by this matrix. A real debugger smoke
test should load `example/debug.py`, stop in `bst.cpp`, and invoke `debug.plot()`.
