#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
	namespace fs = std::filesystem;

	if (argc != 3) {
		std::cerr << "Usage: mkgtk <directory> <namespace>\n";
		return 1;
	}

	const std::string dir = argv[1], ns = argv[2];
	
	try {
		if (fs::exists(dir)) {
			if (!fs::is_directory(dir)) {
				std::cerr << "Error: " << dir << " exists and isn't a directory\n";
				return 2;
			}
		} else
			fs::create_directories(dir);

		fs::path base = dir;
		fs::create_directories(base / "include");
		fs::create_directories(base / "src");

		std::ofstream makefile_stream(base / "Makefile");
		makefile_stream << R"""(ifeq ($(BUILD),release)
BUILDFLAGS := -O3
else
BUILDFLAGS := -g -O0
endif

DEPS       := gtk4 gtkmm-4.0
OUTPUT     := )""" << dir << R"""(
COMPILER   ?= g++
CPPFLAGS   := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude
INCLUDES   := $(shell pkg-config --cflags $(DEPS))
LIBS       := $(shell pkg-config --libs   $(DEPS))
LDFLAGS    := $(LIBS) -pthread
SOURCES    := $(shell find src -name \*.cpp) src/resources.cpp
OBJECTS    := $(SOURCES:.cpp=.o)
RESXML     := $(OUTPUT).gresource.xml
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean test

all: $(OUTPUT)
	./$(OUTPUT)

src/resources.cpp: $(RESXML) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies $(RESXML))
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

%.o: %.cpp
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(CPP) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

clean:
	@ echo rm -f $$\(OBJECTS\) $(OUTPUT) src/resources.cpp
	@ rm -f $(OBJECTS) $(OUTPUT) src/resources.cpp

count:
	cloc . --exclude-dir=.vscode

countbf:
	cloc --by-file . --exclude-dir=.vscode

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -- $(CPP) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
)""";
		makefile_stream.close();
	} catch (const fs::filesystem_error &err) {
		std::cerr << err.what() << "\n";
		return err.code().value();
	} catch (const std::exception &err) {
		std::cerr << err.what() << "\n";
		return 1;
	}
}
