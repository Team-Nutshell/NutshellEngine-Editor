#pragma once
#include "../utils/ntshengn_defines.h"
#include <vector>
#include <string>

namespace NtshEngn {

	struct ProfilerResultNode {
		std::string name = "";

		uint32_t count = 0;
		double totalTime = 0.0;
		double meanTime = 0.0;
		double medianTime = 0.0;
		uint32_t minTimeIndex = 0;
		double minTime = 0.0;
		uint32_t maxTimeIndex = 0;
		double maxTime = 0.0;

		std::vector<ProfilerResultNode> children;

		static std::string to_string(const ProfilerResultNode& resultNode) {
			return to_string(resultNode, 0, false);
		}

		static std::string to_string(const ProfilerResultNode& resultNode, size_t indentationLevel, bool indentFirst) {
			std::string resultNodeString = indentFirst ? std::string(indentationLevel, '\t') : "";

			resultNodeString += resultNode.name + ": Count: " + std::to_string(resultNode.count) + ", Total: " + std::to_string(resultNode.totalTime) + "ms, Median: " + std::to_string(resultNode.medianTime) + "ms, Mean: " + std::to_string(resultNode.meanTime) + "ms, Min: " + std::to_string(resultNode.minTime) + "ms (" + std::to_string(resultNode.minTimeIndex) + "), Max: " + std::to_string(resultNode.maxTime) + "ms (" + std::to_string(resultNode.maxTimeIndex) + ").\n";
			for (const ProfilerResultNode& child : resultNode.children) {
				resultNodeString += to_string(child, indentationLevel + 1, true);
			}

			return resultNodeString;
		}
	};

	class ProfilerInterface {
	public:
		virtual void start(const std::string& sessionName) = 0;

		virtual ProfilerResultNode getResults() = 0;

		virtual ProfilerResultNode end() = 0;

		virtual bool isRunning() const = 0;

		virtual void startBlock(const std::string& blockName) = 0;
		virtual void endBlock() = 0;
	};

}