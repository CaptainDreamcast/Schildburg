#include "overworldplayer.h"

#include <prism/blitz.h>

#include "tilebg.h"
#include "npchandler.h"
#include "fighthandler.h"

static struct {
	Vector3DI mPlayerStart;
	Vector3DI mPlayerStartFacing;

	Vector3DI mPlayerPosition[3];
	Vector3DI mPlayerTarget[3];
	Vector3DI mPlayerFaceDirection[3];


	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	int mEntityID[3];
	int mIsActive[3];

	int mFacingOffset[3];
	int mIsWalking;
	int mIsWalkingPrev;
	int mHasPrincess;

	int mIsWalkActive;
} gOverworldPlayerData;

static void updatePlayerDirectionOffset(int i, Vector3DI tDirection) {
	if (tDirection.x == 0 && tDirection.y == 1) {
		gOverworldPlayerData.mFacingOffset[i] = 0;
	}
	else if (tDirection.x == 0 && tDirection.y == -1) {
		gOverworldPlayerData.mFacingOffset[i] = 1;
	}
	else if (tDirection.x == 1 && tDirection.y == 0) {
		gOverworldPlayerData.mFacingOffset[i] = 2;
	}
	else {
		gOverworldPlayerData.mFacingOffset[i] = 3;
	}

	gOverworldPlayerData.mPlayerFaceDirection[i] = tDirection;
}

static void loadPlayer(void* tData) {
	(void)tData;
	gOverworldPlayerData.mSprites = loadMugenSpriteFileWithoutPalette("player/PLAYER.sff");
	gOverworldPlayerData.mAnimations = loadMugenAnimationFile("player/PLAYER.air");

	for (int i = 0; i < 3; i++) {
		gOverworldPlayerData.mPlayerPosition[i] = gOverworldPlayerData.mPlayerStart + makeVector3DI(i, 0, 0);
		gOverworldPlayerData.mEntityID[i] = addBlitzEntity(makePosition(gOverworldPlayerData.mPlayerPosition[i].x * 16 + 8, gOverworldPlayerData.mPlayerPosition[i].y * 16 + 8, 10));
		updatePlayerDirectionOffset(i, gOverworldPlayerData.mPlayerStartFacing);
		addBlitzMugenAnimationComponent(gOverworldPlayerData.mEntityID[i], &gOverworldPlayerData.mSprites, &gOverworldPlayerData.mAnimations, ((i + 1) * 100) + gOverworldPlayerData.mFacingOffset[i]);
	}

	if (!gOverworldPlayerData.mHasPrincess) {
		setBlitzMugenAnimationVisibility(gOverworldPlayerData.mEntityID[2], 0);
	}

	initPlayerTiles(gOverworldPlayerData.mPlayerPosition[0]);

	setBlitzCameraHandlerPosition(getBlitzEntityPosition(gOverworldPlayerData.mEntityID[0]) - makePosition(160, 120, 0));

	gOverworldPlayerData.mIsWalking = gOverworldPlayerData.mIsWalkingPrev = 0;
	gOverworldPlayerData.mIsWalkActive = 1;
}

static void updatePlayerMovement() {
	if (!gOverworldPlayerData.mIsWalking) return;

	for (int i = 0; i < 3; i++) {
		Vector3DI delta = gOverworldPlayerData.mPlayerTarget[i] - gOverworldPlayerData.mPlayerPosition[i];
		double speed = hasPressedB() ? 2 : 1;

		addBlitzEntityPositionX(gOverworldPlayerData.mEntityID[i], delta.x * speed);
		addBlitzEntityPositionY(gOverworldPlayerData.mEntityID[i], delta.y * speed);
		Position* pos = getBlitzEntityPositionReference(gOverworldPlayerData.mEntityID[i]);
		pos->z = 10 + (pos->y / (16 * 100));
	}

	Vector3DI checkDelta = gOverworldPlayerData.mPlayerTarget[0] - gOverworldPlayerData.mPlayerPosition[0];
	Position p = getBlitzEntityPosition(gOverworldPlayerData.mEntityID[0]);
	Position targetPos = makePosition(gOverworldPlayerData.mPlayerTarget[0].x * 16 + 8, gOverworldPlayerData.mPlayerTarget[0].y * 16 + 8, 0);


	Vector3D targetDelta = targetPos - p;
	if ((checkDelta.x > 0 && targetDelta.x < 0) || (checkDelta.x < 0 && targetDelta.x > 0) || (checkDelta.y > 0 && targetDelta.y < 0) || (checkDelta.y < 0 && targetDelta.y > 0)) {
		for (int i = 0; i < 3; i++) {
			targetPos = makePosition(gOverworldPlayerData.mPlayerTarget[i].x * 16 + 8, gOverworldPlayerData.mPlayerTarget[i].y * 16 + 8, 0);
			targetPos.z = p.z;
			setBlitzEntityPosition(gOverworldPlayerData.mEntityID[i], targetPos);
			gOverworldPlayerData.mPlayerPosition[i] = gOverworldPlayerData.mPlayerTarget[i];
		}
		checkPlayerTeleport(gOverworldPlayerData.mPlayerPosition[0]);
		rollFightStart(getLevelFightProbability());
		updatePlayerTiles(gOverworldPlayerData.mPlayerPosition[0], checkDelta);
		gOverworldPlayerData.mIsWalking = 0;
	}

	setBlitzCameraHandlerPosition(getBlitzEntityPosition(gOverworldPlayerData.mEntityID[0]) - makePosition(160, 120, 0));
}

static void setPlayerMovemement(Vector3DI tDirection, Vector3DI tFacing) {
	updatePlayerDirectionOffset(0, tFacing);
	Vector3DI target = gOverworldPlayerData.mPlayerPosition[0] + tDirection;
	if (isTileBlocked(target) || hasTileNPC(target)) return;

	for (int i = 1; i < 3; i++) {
		Vector3DI direction2 = gOverworldPlayerData.mPlayerPosition[i - 1] - gOverworldPlayerData.mPlayerPosition[i];
		updatePlayerDirectionOffset(i, direction2);
		gOverworldPlayerData.mPlayerTarget[i] = gOverworldPlayerData.mPlayerPosition[i - 1];
	}

	gOverworldPlayerData.mPlayerTarget[0] = target;
	gOverworldPlayerData.mIsWalking = 1;
}

static void updatePlayerMovementInput() {
	if (!gOverworldPlayerData.mIsWalkActive) return;
	if (gOverworldPlayerData.mIsWalking) return;

	Vector3DI direction = makeVector3DI(0, 0, 0);

	if (hasPressedLeft()) {
		direction = makeVector3DI(-1, 0, 0);
	}
	else if (hasPressedRight()) {
		direction = makeVector3DI(1, 0, 0);
	}

	if (hasPressedUp()) {
		direction = makeVector3DI(0, -1, 0);
	}
	else if (hasPressedDown()) {
		direction = makeVector3DI(0, 1, 0);
	}

	if (!direction.x && !direction.y) return;

	setPlayerMovemement(direction, direction);
}

static void updatePlayerAnimation() {

	for (int i = 0; i < 3; i++) {
		int baseAnimation = ((i + 1) * 100) + ((gOverworldPlayerData.mIsWalking ||gOverworldPlayerData.mIsWalkingPrev) ? 20 : 0);
		changeBlitzMugenAnimationIfDifferent(gOverworldPlayerData.mEntityID[i], baseAnimation + gOverworldPlayerData.mFacingOffset[i]);
	}

	gOverworldPlayerData.mIsWalkingPrev = gOverworldPlayerData.mIsWalking;
}

static void updatePlayer(void* tData) {
	(void)tData;
	updatePlayerMovementInput();
	updatePlayerMovement();
	updatePlayerAnimation();
}

ActorBlueprint getOverworldPlayerBP()
{

	return makeActorBlueprint(loadPlayer, NULL, updatePlayer);
}

void resetPlayer()
{
	gOverworldPlayerData.mHasPrincess = 0;
}

void setOverworldPlayerStart(Vector3DI tPosition, Vector3DI tFacing)
{
	gOverworldPlayerData.mPlayerStart = tPosition;
	gOverworldPlayerData.mPlayerStartFacing = tFacing;
}

Vector3DI getPlayerTile()
{
	return gOverworldPlayerData.mPlayerPosition[0];
}

Vector3DI getPlayerFaceDirection()
{
	return gOverworldPlayerData.mPlayerFaceDirection[0];
}

MugenSpriteFile * getPlayerSprites()
{
	return &gOverworldPlayerData.mSprites;
}

MugenAnimations * getPlayerAnimations()
{
	return &gOverworldPlayerData.mAnimations;
}

void setPlayerFaceDirection(int i, Vector3DI tDirection)
{
	updatePlayerDirectionOffset(i, tDirection);
}

void switchPrincessActive()
{
		setBlitzMugenAnimationVisibility(gOverworldPlayerData.mEntityID[2], !gOverworldPlayerData.mHasPrincess);
		gOverworldPlayerData.mHasPrincess = !gOverworldPlayerData.mHasPrincess;
		setFightPrincessActivity(gOverworldPlayerData.mHasPrincess);
}

void setPlayerWalking(Vector3DI tDirection)
{
	Vector3DI facing = tDirection;
	if (facing.x) facing.x /= abs(facing.x);
	if (facing.y) facing.y /= abs(facing.y);

	setPlayerMovemement(tDirection, facing);
}

int isPlayerWalking()
{
	return gOverworldPlayerData.mIsWalking;
}

void setPlayerInactive()
{
	gOverworldPlayerData.mIsWalkActive = 0;
}

void setPlayerActive()
{
	gOverworldPlayerData.mIsWalkActive = 1;
}

int getPlayerActive()
{
	return gOverworldPlayerData.mIsWalkActive;
}

void restorePlayerAfterDeath()
{
	for (int i = 0; i < 3; i++) {
		gOverworldPlayerData.mPlayerPosition[i] = gOverworldPlayerData.mPlayerStart + makeVector3DI(i, 0, 0);
		setBlitzEntityPosition(gOverworldPlayerData.mEntityID[i], makePosition(gOverworldPlayerData.mPlayerPosition[i].x * 16 + 8, gOverworldPlayerData.mPlayerPosition[i].y * 16 + 8, 10));
		updatePlayerDirectionOffset(i, gOverworldPlayerData.mPlayerStartFacing);
	} 

	restoreCharacterHealth();
	setBlitzCameraHandlerPosition(getBlitzEntityPosition(gOverworldPlayerData.mEntityID[0]) - makePosition(160, 120, 0));
	initPlayerTiles(gOverworldPlayerData.mPlayerPosition[0]);
}

void forceStartPosition(Vector3DI tTile, Vector3DI tFacing)
{
	gOverworldPlayerData.mPlayerStart = tTile;
	gOverworldPlayerData.mPlayerStartFacing = tFacing;
}
