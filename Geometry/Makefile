build: test_simple test_simple_opt test_ubsan

test_simple: geometry_test.cpp geometry.h
	clang++ -std=c++20 -gdwarf-4 -O0 -Wall -Wextra -Werror -o ./test_simple geometry_test.cpp geometry.cpp

test_simple_opt: geometry_test.cpp geometry.h
	clang++ -std=c++20 -O2 -Wall -Wextra -Werror -o ./test_simple_opt geometry_test.cpp geometry.cpp

test_ubsan: geometry_test.cpp geometry.h
	clang++ -std=c++20 -g -O0 -Wall -Wextra -Werror -fsanitize=undefined -o ./test_ubsan geometry_test.cpp geometry.cpp

info:
	clang++ --version
	clang-tidy --version
	clang-format --version
	valgrind --version

run: build
	@echo 'Run tests (simple)'
	time ./test_simple
	@echo 'Run tests (simple_opt)'
	time ./test_simple_opt
	@echo 'Run tests (ubsan)'
	time ./test_ubsan
	@echo 'Run tests (valgrind)'
	time valgrind --leak-check=yes ./test_simple 

lint:
	@echo 'Check code is formatted'
	clang-format --style=file --dry-run --Werror *.h *.cpp
	@echo 'Run linter'
	clang-tidy --config "$(shell cat .clang-tidy)" --warnings-as-errors="*"  geometry_test.cpp geometry.cpp '-header-filter=.*' -- -std=c++20 -g -O0 -Wall -Wextra -Werror
	@echo 'Check NOLINT is not used'
	! grep NOLINT geometry.h geometry.cpp
	@echo 'Check all TODOs are removed'
	! grep TODO geometry.h geometry.cpp

test: info run lint
	@echo 'Great job!'

format:
	@echo 'Apply linter fixes'
	clang-tidy --config "$(shell cat .clang-tidy)" --fix geometry_test.cpp geometry.cpp '-header-filter=.*' -- -std=c++20 -g -O0 -Wall -Wextra -Werror
	@echo 'Apply formatter'
	clang-format --style=file -i *.h *.cpp

clean:
	rm test_simple test_simple_opt test_ubsan
