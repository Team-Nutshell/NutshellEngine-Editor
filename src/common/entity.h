#pragma once
#include <string>
#include <cstdint>

typedef uint32_t EntityID;
#define NO_ENTITY 0xFFFFFFFF

struct Entity {
	EntityID entityID = NO_ENTITY;
	std::string name = "";

	bool isVisible = true;
};