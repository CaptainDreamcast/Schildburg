#pragma once

#include <prism/actorhandler.h>

ActorBlueprint getFightHandler();

void rollFightStart(double tLikeliness);
void resetFightHandler();
void setFightPrincessActivity(int tIsActive);
void restoreCharacterHealth();
void startFinalFight();