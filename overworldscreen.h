#pragma once

#include <string>

#include <prism/wrapper.h>
#include <prism/geometry.h>

Screen* getOverworldScreen();
void setNewOverworldData(std::string tName, Vector3DI tPlayerStart, Vector3DI tPlayerStartFacing);