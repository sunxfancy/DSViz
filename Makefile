all: basic bst

%: example/%.cpp
	mkdir -p bin && clang++ -glldb -std=c++11 -I. ./example/$*.cpp -o bin/$*

clean:
	rm -rf bin