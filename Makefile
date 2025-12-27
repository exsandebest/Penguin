.PHONY: format lint run_tests

format:
	clang-format -i src/*.cpp src/*.h

lint:
	clang-format --dry-run --Werror src/*.cpp src/*.h

run_tests:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --config Release
	ctest --test-dir build
	PENGUIN_EXE=./build/Penguin EXAMPLES_DIR=examples bash scripts/tests.sh
