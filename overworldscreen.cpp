#include "overworldscreen.h"

#include <prism/blitz.h>

#include "tilebg.h"
#include "overworldplayer.h"
#include "npchandler.h"
#include "fighthandler.h"
#include "storyhandler.h"

static void loadOverworldScreen() {
	instantiateActor(getStoryHandler());
	instantiateActor(getNPCHandler());
	instantiateActor(getTileBGHandler());
	instantiateActor(getOverworldPlayerBP());
	resetFightHandler();
	instantiateActor(getFightHandler());
}

static void updateOverWorldScreen() {
	
}

static Screen gOverworldScreen;

Screen * getOverworldScreen()
{
	gOverworldScreen = makeScreen(loadOverworldScreen, updateOverWorldScreen);
	return &gOverworldScreen;
}

void setNewOverworldData(string tName, Vector3DI tPlayerStart, Vector3DI tPlayerStartFacing)
{
	setTileBGName(tName);
	setOverworldPlayerStart(tPlayerStart, tPlayerStartFacing);
}
