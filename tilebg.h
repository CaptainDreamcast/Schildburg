#pragma once

#include <string>

#include <prism/actorhandler.h>
#include <prism/mugenanimationhandler.h>

ActorBlueprint getTileBGHandler();

void setTileBGName(std::string tName);
std::string getTileBGName();
MugenSpriteFile* getLevelSprites();
MugenAnimations* getLevelAnimations();
int isTileBlocked(Vector3DI tLocation);
void checkPlayerTeleport(Vector3DI tLocation);
void initPlayerTiles(Vector3DI tPos);
void updatePlayerTiles(Vector3DI tPos, Vector3DI tDelta);
double getLevelFightProbability();