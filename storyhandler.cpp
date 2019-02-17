#include "storyhandler.h"

#include <prism/blitz.h>

#include "overworldplayer.h"

#include "tilebg.h"
#include "npchandler.h"
#include "fighthandler.h"
#include "titlescreen.h"

static struct {
	int mCurrentStep;

	vector<Buffer> mStories;
	
	BufferPointer mP;
	int mIsActive;
	int mIsLineActive;

	int mBGID;

} gStoryHandler;

static void loadStoryHandler(void* tData) {
	(void)tData;

	gStoryHandler.mIsActive = 0;

	gStoryHandler.mStories.clear();
	for (int i = 0; i < 8; i++) {
		char text[100];
		sprintf(text, "stories/%d.txt", i);
		gStoryHandler.mStories.push_back(fileToBuffer(text));
	}
}

static void startStory(int tID) {

	setPlayerInactive();
	gStoryHandler.mP = getBufferPointer(gStoryHandler.mStories[tID]);
	gStoryHandler.mIsLineActive = 0; 
	gStoryHandler.mIsActive = 1;
	
}

static void updateStoryHandlerActive() {
	if (gStoryHandler.mIsActive) return;
	if (!getPlayerActive()) return;
	if (isPlayerWalking()) return;

	Vector3DI playerPos = getPlayerTile();

	if (gStoryHandler.mCurrentStep == 0) {
		startStory(0);
	}

	if (gStoryHandler.mCurrentStep == 1 && vecEqualsI2D(playerPos, makeVector3DI(10, 7, 0))) {
		if (hasPressedAFlank()) {
			startStory(1);
		}
	}

	if (gStoryHandler.mCurrentStep == 1 && playerPos.y == 19) {
		startStory(2);
	}

	if (gStoryHandler.mCurrentStep == 2 && getTileBGName() == "overworld" && vecEqualsI2D(playerPos, makeVector3DI(21, 19, 0))) {
		startStory(3);
		forceStartPosition(playerPos, makeVector3DI(1, 0, 0));
	}

	if (gStoryHandler.mCurrentStep == 3 && getTileBGName() == "castle3" && vecEqualsI2D(playerPos, makeVector3DI(02, 06, 0))) {
		if (hasPressedAFlank()) {
			startStory(4);
		}
	}

	if (gStoryHandler.mCurrentStep == 4 && getTileBGName() == "castle3" && vecEqualsI2D(playerPos, makeVector3DI(02, 06, 0))) {
		if (hasPressedAFlank()) {
			startStory(5);
		}
	}

	if (gStoryHandler.mCurrentStep == 4 && playerPos.y == 23) {
		startStory(6);
	}

	if (gStoryHandler.mCurrentStep == 5) {
		startStory(7);
	}
}

static void setStoryHandlerInactive(int tIncrease) {
	gStoryHandler.mCurrentStep += tIncrease;
	gStoryHandler.mIsActive = 0;
}

static void updateStoryHandlerAction();

static void storyTextFinished() {
	gStoryHandler.mIsLineActive = 0;
	updateStoryHandlerAction();
}

static void gotoTitle(void* tCaller) {
	(void)tCaller;
	setNewScreen(getTitleScreen());
}

static void updateStoryHandlerAction() {
	while (gStoryHandler.mIsActive && !gStoryHandler.mIsLineActive) {
		string line = readLineFromTextStreamBufferPointer(&gStoryHandler.mP);
		if (line[0] == '&') {
			gStoryHandler.mIsLineActive = 0;
			setPlayerActive();
			setStoryHandlerInactive(1);
		} else 	if (line[0] == '!') {
			gStoryHandler.mIsLineActive = 0;
			setPlayerActive();
			setStoryHandlerInactive(0);
		}
		else if (line[0] == '(') {
			gStoryHandler.mIsLineActive = 0;
			gStoryHandler.mBGID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 1000), getLevelSprites(), makePosition(0, 0, 20));
			setMugenAnimationDrawSize(gStoryHandler.mBGID, makePosition(320, 240, 1));
		}
		else if (line[0] == '[') {
			gStoryHandler.mIsLineActive = 0;
			gStoryHandler.mBGID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 1001), getLevelSprites(), makePosition(0, 0, 20));
			setMugenAnimationDrawSize(gStoryHandler.mBGID, makePosition(320, 240, 1));
		}
		else if (line[0] == ')' || line[0] == ']') {
			gStoryHandler.mIsLineActive = 0;
			removeMugenAnimation(gStoryHandler.mBGID);
		}
		else if (line[0] == '$') {
			char dummy[10];
			int id, animation, speed;
			Vector3DI target = makeVector3DI(0, 0, 0);
			sscanf(line.data(), "%s %d %d %d %d %d", dummy, &id, &animation, &target.x, &target.y, &speed);
			gStoryHandler.mIsLineActive = 1;
			setNPCWalking(id, animation, target, speed, storyTextFinished);
		}
		else if (line[0] == '*') {
			char dummy[10];
			Vector3DI direction = makeVector3DI(0, 0, 0);
			sscanf(line.data(), "%s %d %d", dummy, &direction.x, &direction.y);
			gStoryHandler.mIsLineActive = 0;
			setPlayerWalking(direction);
		}
		else if (line[0] == '=') {
			char dummy[10];
			int id;
			Vector3DI faceDirection = makeVector3DI(0, 0, 0);
			sscanf(line.data(), "%s %d %d %d", dummy, &id, &faceDirection.x, &faceDirection.y);
			setPlayerFaceDirection(id, faceDirection);
			gStoryHandler.mIsLineActive = 0;
		}
		else if (line[0] == '-') {
			char dummy[10];
			int id, visibility;
			sscanf(line.data(), "%s %d %d", dummy, &id, &visibility);
			setNPCVisibility(id, visibility);
		}
		else if (line[0] == '+') {
			switchPrincessActive();
		}
		else if (line[0] == '.') {
			setStoryHandlerInactive(1);
			startFinalFight();
		}
		else if (line[0] == ',') {
			setStoryHandlerInactive(0);
			startFinalFight();
		}
		else if (line[0] == '<') {
			gStoryHandler.mBGID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 5007), getLevelSprites(), makePosition(0, 0, 20));
		}
		else if (line[0] == '>') {
			gStoryHandler.mIsLineActive = 1;
			addFadeOut(180, gotoTitle, NULL);
		}
		else if (line[0] == '/') {
			char dummy[10];
			int id, animation;
			sscanf(line.data(), "%s %d %d", dummy, &id, &animation);
			setNPCAnimation(id, animation);
		}
		else if (line.find(':') != line.npos) {
			int sep = line.find(':');
			string name = line.substr(0, line.find(":"));
			string text = line.substr(line.find(":") + 2);
			setTextboxActiveExternal(name, text, storyTextFinished);
			gStoryHandler.mIsLineActive = 1;
		}
	}
}

static void updateStoryHandler(void* tData) {
	(void)tData;
	updateStoryHandlerActive();
	updateStoryHandlerAction();
}


ActorBlueprint getStoryHandler()
{
	return makeActorBlueprint(loadStoryHandler, NULL, updateStoryHandler);
}

void resetStory()
{
	resetPlayer();
	gStoryHandler.mCurrentStep = 0;
}

void increaseStoryPart()
{
	gStoryHandler.mCurrentStep++;
}
