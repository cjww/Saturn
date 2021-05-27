#include <iostream>
#include <filesystem>


int main() {

	namespace fs = std::filesystem;

	fs::path srcPath = "C:/Users/juliu/source/repos/Saturn/Engine/src";
	fs::path includePath = "C:/Users/juliu/source/repos/Saturn/Engine/include/Saturn";

	std::cout << srcPath << std::endl;
	std::cout << includePath << std::endl;
	std::cout << std::endl;

	fs::recursive_directory_iterator it(srcPath);
	for (const auto& file : it) {
		fs::path name = fs::relative(file.path(), srcPath);
		
		fs::path corrPath = includePath;
		corrPath.append(name.c_str());
		if (!fs::exists(corrPath)) {
			if (fs::is_directory(file.path())) {
				fs::create_directory(corrPath);
			}
			else if (name.generic_string().find_last_of(".h") == name.generic_string().size() - 1) {
				fs::copy(file.path(), corrPath);
			}
		}


	}


	return 0;
}