COMPILER ?= g++

.PHONY: clean test

mkgtk: mkgtk.cpp
	$(COMPILER) -std=c++20 $< -o $@

clean:
	rm -rf mkgtk test

test: mkgtk
	./mkgtk test com.heimskr.test Test
