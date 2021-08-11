#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
	namespace fs = std::filesystem;

	if (argc != 4) {
		std::cerr << "Usage: mkgtk <directory> <namespace> <prefix>\n";
		return 1;
	}

	const std::string dir = argv[1], prefix = argv[2], ns = argv[3];

	std::string slash_prefix = "/" + prefix;
	std::replace(slash_prefix.begin(), slash_prefix.end(), '.', '/');
	
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

		std::ofstream makefile(base / "Makefile");
		makefile << R"""(ifeq ($(BUILD),release)
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
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)
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
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
)""";
		makefile.close();

		std::ofstream gitignore(base / ".gitignore");
		gitignore << "/" << dir << "\n*.o\n/.dep\n.vscode\n/src/resources.cpp\n";
		gitignore.close();

		std::ofstream gresource(base / (dir + ".gresource.xml"));
		gresource << R"""(<?xml version="1.0" encoding="UTF-8"?>
<gresources>
	<gresource prefix=")""" << slash_prefix << R"""(">
		<file preprocess="xml-stripblanks">window.ui</file>
		<file>style.css</file>
	</gresource>
</gresources>
)""";
		gresource.close();

		std::ofstream window(base / "window.ui");
		window << R"""(<?xml version="1.0" encoding="UTF-8"?>
<interface>
	<requires lib="gtk+" version="4.0" />
	<object class="GtkApplicationWindow" id="app_window">
		<property name="title" translatable="yes">)""" << ns << R"""(</property>
		<property name="default-width">1600</property>
		<property name="default-height">1000</property>
		<property name="hide-on-close">True</property>
		<child type="titlebar">
			<object class="GtkHeaderBar" id="headerbar">
				<property name="title-widget">
					<object class="GtkLabel">
						<property name="label" translatable="yes">)""" << ns << R"""(</property>
						<property name="single-line-mode">True</property>
						<property name="ellipsize">end</property>
						<style>
							<class name="title" />
						</style>
					</object>
				</property>
				<child type="end">
					<object class="GtkMenuButton" id="menu_button">
						<property name="valign">center</property>
						<property name="focus-on-click">0</property>
						<property name="menu-model">menu</property>
						<property name="icon-name">open-menu-symbolic</property>
						<accessibility>
							<property name="label" translatable="yes">Primary menu</property>
						</accessibility>
					</object>
				</child>
			</object>
		</child>
	</object>
	<menu id="menu">
		<section>
			<!-- <item>
				<attribute name="label" translatable="yes">_Example</attribute>
				<attribute name="action">win.example</attribute>
			</item> -->
		</section>
	</menu>
</interface>
)""";
		window.close();

		std::ofstream(base / "style.css").close();
	} catch (const fs::filesystem_error &err) {
		std::cerr << err.what() << "\n";
		return err.code().value();
	} catch (const std::exception &err) {
		std::cerr << err.what() << "\n";
		return 1;
	}
}
