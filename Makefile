.PHONY: format lint

format:
	clang-format -i src/*.cpp src/*.h

lint:
	clang-format --dry-run --Werror src/*.cpp src/*.h
