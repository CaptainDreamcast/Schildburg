#pragma once

#include <string>

#include <prism/actorhandler.h>
#include <prism/file.h>
#include <prism/mugenanimationhandler.h>
#include <prism/geometry.h>

ActorBlueprint getNPCHandler();

void loadNPCs(std::string tPath, MugenSpriteFile* tSprites, MugenAnimations* tAnimations);
void setTextboxActiveExternal(std::string tName, std::string tTextbox, void(*tCB)());
int hasTileNPC(Vector3DI tTarget);
void setNPCWalking(int tID, int tAnimation, Vector3DI tTarget, int tSpeed, void(*tCB)());
void setNPCVisibility(int tID, int tVisibility);
void setNPCAnimation(int tID, int tAnimation);
