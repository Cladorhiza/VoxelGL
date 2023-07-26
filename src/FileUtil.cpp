#include "FileUtil.h"

#include <fstream>
#include <sstream>
#include <string>

namespace FileUtil {


	std::string ParseFileToString(const std::string& filepath) {

		std::ifstream stream(filepath);

		std::string line;
		std::stringstream ss;
		while (getline(stream, line)) {
			ss << line << '\n';
		}
		return { ss.str() };
	}
}
