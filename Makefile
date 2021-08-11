COMPILER ?= g++

.PHONY: clean test

mkgtk: mkgtk.cpp
	$(COMPILER) -std=c++20 $< -o $@

clean:
	rm mkgtk

test: mkgtk
	./mkgtk test Test
