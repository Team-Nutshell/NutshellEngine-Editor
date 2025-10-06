#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace NtshEngn {

	class CommandLineInterface {
	public:
		virtual void set(int argc, char** argv) = 0;
		virtual const std::unordered_map<std::string, std::vector<std::string>>& get() = 0;
	};

}