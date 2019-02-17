#include "titlescreen.h"

#include <prism/blitz.h>

#include "storyhandler.h"
#include "overworldplayer.h"
#include "fighthandler.h"
#include "overworldscreen.h"


static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;

	int mPressStart;

} gTitleScreenData;

static void loadTitleScreen() {
	gTitleScreenData.mSprites = loadMugenSpriteFileWithoutPalette("data/TITLE.sff");
	gTitleScreenData.mAnimations = loadMugenAnimationFile("data/TITLE.air");

	addMugenAnimation(getMugenAnimation(&gTitleScreenData.mAnimations, 1), &gTitleScreenData.mSprites, makePosition(0, 0, 1));
	gTitleScreenData.mPressStart = addMugenAnimation(getMugenAnimation(&gTitleScreenData.mAnimations, 2), &gTitleScreenData.mSprites, makePosition(160, 102, 2));
	addFadeIn(20, NULL, NULL);
}

static void gotoGameScreen(void* tCaller) {
	(void)tCaller;
	resetStory();
	setNewOverworldData("castle", makeVector3DI(10, 12, 0), makeVector3DI(0, 1, 0));
	restoreCharacterHealth();
	setNewScreen(getOverworldScreen());
}

static void updateTitleScreen() {
	if (hasPressedStartFlank()) {
		changeMugenAnimation(gTitleScreenData.mPressStart, getMugenAnimation(&gTitleScreenData.mAnimations, 3));
		addFadeOut(20, gotoGameScreen, NULL);
	}
}

Screen gTitleScreen;

Screen * getTitleScreen()
{
	gTitleScreen = makeScreen(loadTitleScreen, updateTitleScreen);
	return &gTitleScreen;
}
