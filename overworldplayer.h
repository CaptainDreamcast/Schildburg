#pragma once

#include <string>
#include <prism/actorhandler.h>
#include <prism/geometry.h>
#include <prism/mugenanimationhandler.h>

ActorBlueprint getOverworldPlayerBP();

void resetPlayer();
void setOverworldPlayerStart(Vector3DI tPosition, Vector3DI tFacing);
Vector3DI getPlayerTile();
Vector3DI getPlayerFaceDirection();
MugenSpriteFile* getPlayerSprites();
MugenAnimations* getPlayerAnimations();
void setPlayerFaceDirection(int i, Vector3DI tDirection);
void switchPrincessActive();
void setPlayerWalking(Vector3DI tDirection);
int isPlayerWalking();
void setPlayerInactive();
void setPlayerActive();
int getPlayerActive();
void restorePlayerAfterDeath();
void forceStartPosition(Vector3DI tTile, Vector3DI tFacing);