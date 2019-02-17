#include "tilebg.h"

#include <prism/blitz.h>

#include "npchandler.h"
#include "overworldplayer.h"
#include "overworldscreen.h"

typedef struct {
	string mDestinationName;
	Vector3DI mDestinationTile;
	Vector3DI mDestinationFacing;

} Teleport;

static struct {
	string mName;
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;

	Vector3DI mSize;

	vector<vector<int> > mBG1;
	vector<vector<int> > mBG2;
	vector<vector<int> > mBG3;
	vector<MugenAnimation*> mAnimationList;

	vector<vector<int> > mIsLoaded;
	vector<vector<int> > mBG1IDs;
	vector<vector<int> > mBG2IDs;
	vector<vector<int> > mBG3IDs;

	vector<vector<int> > mIsBlocked;
	map<pair<int, int>, Teleport> mTeleports;

	double mProbability;
} gTileBGHandler;

static void loadTilesFromFile(string tPath) {
	Buffer b = fileToBuffer(tPath.data());
	BufferPointer p = getBufferPointer(b);

	gTileBGHandler.mSize.x = readIntegerFromTextStreamBufferPointer(&p);
	gTileBGHandler.mSize.y = readIntegerFromTextStreamBufferPointer(&p);

	gTileBGHandler.mBG1 = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	for (int j = 0; j < gTileBGHandler.mSize.y; j++) {
		for (int i = 0; i < gTileBGHandler.mSize.x; i++) {
			gTileBGHandler.mBG1[j][i] = readIntegerFromTextStreamBufferPointer(&p);
		}
	}

	gTileBGHandler.mBG2 = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	for (int j = 0; j < gTileBGHandler.mSize.y; j++) {
		for (int i = 0; i < gTileBGHandler.mSize.x; i++) {
			gTileBGHandler.mBG2[j][i] = readIntegerFromTextStreamBufferPointer(&p);
		}
	}

	gTileBGHandler.mBG3 = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	for (int j = 0; j < gTileBGHandler.mSize.y; j++) {
		for (int i = 0; i < gTileBGHandler.mSize.x; i++) {
			gTileBGHandler.mBG3[j][i] = readIntegerFromTextStreamBufferPointer(&p);
		}
	}

	gTileBGHandler.mIsBlocked = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	for (int j = 0; j < gTileBGHandler.mSize.y; j++) {
		for (int i = 0; i < gTileBGHandler.mSize.x; i++) {
			gTileBGHandler.mIsBlocked[j][i] = readIntegerFromTextStreamBufferPointer(&p);
		}
	}

	gTileBGHandler.mIsLoaded = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	gTileBGHandler.mBG1IDs = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	gTileBGHandler.mBG2IDs = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));
	gTileBGHandler.mBG3IDs = vector<vector<int> >(gTileBGHandler.mSize.y, vector<int>(gTileBGHandler.mSize.x, 0));

	gTileBGHandler.mAnimationList.clear();
	for (int i = 0; i < 100; i++) {
		gTileBGHandler.mAnimationList.push_back(createOneFrameMugenAnimationForSprite(1, i));
	}

	freeBuffer(b);
}

static void loadTeleports(string tPath) {
	gTileBGHandler.mTeleports.clear();

	Buffer b = fileToBuffer(tPath.data());
	BufferPointer p = getBufferPointer(b);

	int probInteger = readIntegerFromTextStreamBufferPointer(&p);
	gTileBGHandler.mProbability = probInteger / 100.0;

	int amount = readIntegerFromTextStreamBufferPointer(&p);

	for (int i = 0; i < amount; i++) {
		Vector3DI pos = makeVector3DI(0, 0, 0);
		Teleport e;

		pos.x = readIntegerFromTextStreamBufferPointer(&p);
		pos.y = readIntegerFromTextStreamBufferPointer(&p);
		e.mDestinationName = readStringFromTextStreamBufferPointer(&p);
		e.mDestinationTile.x = readIntegerFromTextStreamBufferPointer(&p);
		e.mDestinationTile.y = readIntegerFromTextStreamBufferPointer(&p);
		e.mDestinationTile.z = 0;
		e.mDestinationFacing.x = readIntegerFromTextStreamBufferPointer(&p);
		e.mDestinationFacing.y = readIntegerFromTextStreamBufferPointer(&p);
		e.mDestinationFacing.z = 0;
		gTileBGHandler.mTeleports[make_pair(pos.x, pos.y)] = e;
	}
}

static void loadTileBGHandler(void* tData) {
	(void)tData;
	
	string path = "levels/" + gTileBGHandler.mName + "/LEVEL.sff";
	gTileBGHandler.mSprites = loadMugenSpriteFileWithoutPalette(path);

	path = "levels/" + gTileBGHandler.mName + "/LEVEL.air";
	gTileBGHandler.mAnimations = loadMugenAnimationFile(path);

	path = "levels/" + gTileBGHandler.mName + "/TILES.txt";
	loadTilesFromFile(path);

	path = "levels/" + gTileBGHandler.mName + "/NPCS.txt";
	loadNPCs(path, &gTileBGHandler.mSprites, &gTileBGHandler.mAnimations);

	path = "levels/" + gTileBGHandler.mName + "/TELEPORTS.txt";
	loadTeleports(path);
}

ActorBlueprint getTileBGHandler()
{
	return makeActorBlueprint(loadTileBGHandler);
}

void setTileBGName(string tName)
{
	gTileBGHandler.mName = tName;
}

std::string getTileBGName()
{
	return gTileBGHandler.mName;
}

MugenSpriteFile * getLevelSprites()
{
	return &gTileBGHandler.mSprites;
}

MugenAnimations * getLevelAnimations()
{
	return &gTileBGHandler.mAnimations;
}

int isTileBlocked(Vector3DI tLocation)
{
	if (tLocation.x < 0 || tLocation.y < 0 || tLocation.x >= gTileBGHandler.mSize.x || tLocation.y >= gTileBGHandler.mSize.y) return 1;
	return gTileBGHandler.mIsBlocked[tLocation.y][tLocation.x];
}

void checkPlayerTeleport(Vector3DI tLocation)
{
	auto it = gTileBGHandler.mTeleports.find(make_pair(tLocation.x, tLocation.y));
	if (it == gTileBGHandler.mTeleports.end()) return;

	setPlayerInactive();
	setNewOverworldData(it->second.mDestinationName, it->second.mDestinationTile, it->second.mDestinationFacing);
	setNewScreen(getOverworldScreen());
}

static void loadSingleTile(int i, int j) {
	if (gTileBGHandler.mIsLoaded[j][i]) return;

	int value = gTileBGHandler.mBG1[j][i];
	if (value) {
		Position pos = makePosition(i * 16, j * 16, 2);
		gTileBGHandler.mBG1IDs[j][i] = addMugenAnimation(gTileBGHandler.mAnimationList[value], &gTileBGHandler.mSprites, pos);
		setMugenAnimationCameraPositionReference(gTileBGHandler.mBG1IDs[j][i], getBlitzCameraHandlerPositionReference());
	}

	value = gTileBGHandler.mBG2[j][i];
	if (value) {
		Position pos = makePosition(i * 16, j * 16, 3);
		gTileBGHandler.mBG2IDs[j][i] = addMugenAnimation(gTileBGHandler.mAnimationList[value], &gTileBGHandler.mSprites, pos);
		setMugenAnimationCameraPositionReference(gTileBGHandler.mBG2IDs[j][i], getBlitzCameraHandlerPositionReference());
	}

	value = gTileBGHandler.mBG3[j][i];
	if (value) {
		Position pos = makePosition(i * 16, j * 16, 13);
		gTileBGHandler.mBG3IDs[j][i] = addMugenAnimation(gTileBGHandler.mAnimationList[value], &gTileBGHandler.mSprites, pos);
		setMugenAnimationCameraPositionReference(gTileBGHandler.mBG3IDs[j][i], getBlitzCameraHandlerPositionReference());
	}

	gTileBGHandler.mIsLoaded[j][i] = 1;
}

static void unloadSingleTile(int i, int j) {
	if (!gTileBGHandler.mIsLoaded[j][i]) return;

	int value = gTileBGHandler.mBG1[j][i];
	if (value) {
		removeMugenAnimation(gTileBGHandler.mBG1IDs[j][i]);
	}

	value = gTileBGHandler.mBG2[j][i];
	if (value) {
		removeMugenAnimation(gTileBGHandler.mBG2IDs[j][i]);

	}

	value = gTileBGHandler.mBG3[j][i];
	if (value) {
		removeMugenAnimation(gTileBGHandler.mBG3IDs[j][i]);
	}

	gTileBGHandler.mIsLoaded[j][i] = 0;
}

#define STREAM_WIDTH_X 12
#define STREAM_WIDTH_Y 9

void initPlayerTiles(Vector3DI tPos)
{

	for (int j = tPos.y - STREAM_WIDTH_Y; j < tPos.y + STREAM_WIDTH_Y; j++) {
		if (j < 0) continue;
		if (j >= gTileBGHandler.mSize.y) break;
		for (int i = tPos.x - STREAM_WIDTH_X; i < tPos.x + STREAM_WIDTH_X; i++) {
			if (i < 0) continue;
			if (i >= gTileBGHandler.mSize.x) break;
			loadSingleTile(i, j);
		}
	}
}

void updatePlayerTiles(Vector3DI tPos, Vector3DI tDelta)
{
	tPos = tPos - tDelta;

	if (tDelta.x == 1) {
		for (int j = tPos.y - STREAM_WIDTH_Y; j < tPos.y + STREAM_WIDTH_Y; j++) {
			if (j < 0) continue;
			if (j >= gTileBGHandler.mSize.y) break;
			if (tPos.x + STREAM_WIDTH_X >= 0 && tPos.x + STREAM_WIDTH_X < gTileBGHandler.mSize.x) loadSingleTile(tPos.x + STREAM_WIDTH_X, j);
			if (tPos.x - STREAM_WIDTH_X >= 0 && tPos.x - STREAM_WIDTH_X < gTileBGHandler.mSize.x) unloadSingleTile(tPos.x - STREAM_WIDTH_X, j);
		}
	} else if (tDelta.x == -1) {
		for (int j = tPos.y - STREAM_WIDTH_Y; j < tPos.y + STREAM_WIDTH_Y; j++) {
			if (j < 0) continue;
			if (j >= gTileBGHandler.mSize.y) break;
			if (tPos.x + STREAM_WIDTH_X >= 0 && tPos.x + STREAM_WIDTH_X < gTileBGHandler.mSize.x) unloadSingleTile(tPos.x + STREAM_WIDTH_X, j);
			if (tPos.x - STREAM_WIDTH_X >= 0 && tPos.x - STREAM_WIDTH_X < gTileBGHandler.mSize.x) loadSingleTile(tPos.x - STREAM_WIDTH_X, j);
		}
	} else if (tDelta.y == 1) {
		for (int i = tPos.x - STREAM_WIDTH_X; i < tPos.x + STREAM_WIDTH_X; i++) {
			if (i < 0) continue;
			if (i >= gTileBGHandler.mSize.x) break;
			if (tPos.y + STREAM_WIDTH_Y >= 0 && tPos.y + STREAM_WIDTH_Y < gTileBGHandler.mSize.y) loadSingleTile(i, tPos.y + STREAM_WIDTH_Y);
			if (tPos.y - STREAM_WIDTH_Y >= 0 && tPos.y - STREAM_WIDTH_Y < gTileBGHandler.mSize.y) unloadSingleTile(i, tPos.y - STREAM_WIDTH_Y);
		}
	} else if (tDelta.y == -1) {
		for (int i = tPos.x - STREAM_WIDTH_X; i < tPos.x + STREAM_WIDTH_X; i++) {
			if (i < 0) continue;
			if (i >= gTileBGHandler.mSize.x) break;
			if (tPos.y + STREAM_WIDTH_Y >= 0 && tPos.y + STREAM_WIDTH_Y < gTileBGHandler.mSize.y) unloadSingleTile(i, tPos.y + STREAM_WIDTH_Y);
			if (tPos.y - STREAM_WIDTH_Y >= 0 && tPos.y - STREAM_WIDTH_Y < gTileBGHandler.mSize.y) loadSingleTile(i, tPos.y - STREAM_WIDTH_Y);
		}
	}

}

double getLevelFightProbability()
{
	return gTileBGHandler.mProbability;
}
