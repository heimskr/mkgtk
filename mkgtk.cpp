#include <filesystem>
#include <iostream>

int main(int argc, char **argv) {
	namespace fs = std::filesystem;

	if (argc != 3) {
		std::cerr << "Usage: mkgtk <directory> <namespace>\n";
		return 1;
	}

	const char *dir = argv[1], *ns = argv[2];
	
	try {
		if (fs::exists(dir)) {
			if (!fs::is_directory(dir)) {
				std::cerr << "Error: " << dir << " exists and isn't a directory\n";
				return 2;
			}
		} else
			fs::create_directories(dir);
	} catch (const fs::filesystem_error &err) {
		std::cerr << err.what() << "\n";
		return err.code().value();
	}
}
