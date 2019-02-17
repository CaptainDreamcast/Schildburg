#include "npchandler.h"

#include <prism/blitz.h>

#include "overworldplayer.h"

typedef struct {
	int mID;
	Vector3DI mTile;
	int mEntityID;
	string mName;
	vector<string> mText;

	int mIsWalking;
	Vector3DI mTarget;
	int mSpeed;
	void(*mCB)();

	int mIsVisible;
} NPC;

static struct {
	map<int, NPC> mNPCs;

	int mCanSpeakPrev;
	int mCanSpeak;
	int mSpeakID;
	int mSpeakEntityID;

	int mIsSpeaking;
	int mTextBoxAnimationID;
	int mNameTextID;
	int mTextID;
	int mCurrentTextStep;

	int mIsExternalText;
	void(*mCB)();
} gNPCHandler;

static void loadNPCHandler(void* tData) {
	(void) tData;
	gNPCHandler.mNPCs.clear();
	gNPCHandler.mCanSpeak = gNPCHandler.mCanSpeakPrev = 0;
	gNPCHandler.mIsSpeaking = 0;
	gNPCHandler.mIsExternalText = 0;
}

static void updateSingleNPCCanSpeak(NPC& tNPC) {
	if (!tNPC.mIsVisible) return;
	if (tNPC.mIsWalking) return;
	if (!getPlayerActive()) return;

	Vector3DI faceDirection = getPlayerFaceDirection();
	Vector3DI playerPos = getPlayerTile();
	int isLeft = (playerPos.x == tNPC.mTile.x - 1) && (playerPos.y == tNPC.mTile.y) && faceDirection.x == 1;
	int isRight = (playerPos.x == tNPC.mTile.x + 1) && (playerPos.y == tNPC.mTile.y) && faceDirection.x == -1;
	int isUp = (playerPos.x == tNPC.mTile.x) && (playerPos.y == tNPC.mTile.y - 1) && faceDirection.y == 1;
	int isDown = (playerPos.x == tNPC.mTile.x) && (playerPos.y == tNPC.mTile.y + 1) && faceDirection.y == -1;

	if (isLeft || isRight || isUp || isDown) {
		gNPCHandler.mSpeakID = tNPC.mID;
		gNPCHandler.mCanSpeak = 1;
	}
}

static void updateNPCWalking(NPC& tNPC) {
	if (!tNPC.mIsWalking) return;
	Vector3DI delta = tNPC.mTarget - tNPC.mTile;
	double speed = tNPC.mSpeed;

	if (delta.x) delta.x /= abs(delta.x);
	if (delta.y) delta.y /= abs(delta.y);



	addBlitzEntityPositionX(tNPC.mEntityID, delta.x * speed);
	addBlitzEntityPositionY(tNPC.mEntityID, delta.y * speed);
	Position* pos = getBlitzEntityPositionReference(tNPC.mEntityID);
	pos->z = 10 + (pos->y / (16 * 100));

	Position p = getBlitzEntityPosition(tNPC.mEntityID);
	Position targetPos = makePosition(tNPC.mTarget.x * 16 + 8, tNPC.mTarget.y * 16 + 8, 0);

	Vector3D targetDelta = targetPos - p;
	if ((delta.x > 0 && targetDelta.x < 0) || (delta.x < 0 && targetDelta.x > 0) || (delta.y > 0 && targetDelta.y < 0) || (delta.y < 0 && targetDelta.y > 0)) {
		targetPos.z = p.z;
		setBlitzEntityPosition(tNPC.mEntityID, targetPos);
		tNPC.mTile = tNPC.mTarget;
		tNPC.mCB();
		tNPC.mIsWalking = 0;
	}
}

static void updateSingleNPC(NPC& tNPC) {
	updateSingleNPCCanSpeak(tNPC);
	updateNPCWalking(tNPC);
}

static void setSpeakActive() {
	Vector3D enemyPos = getBlitzEntityPosition(gNPCHandler.mNPCs[gNPCHandler.mSpeakID].mEntityID);
	gNPCHandler.mSpeakEntityID = addBlitzEntity(enemyPos + makePosition(0,0, 10));
	addBlitzMugenAnimationComponent(gNPCHandler.mSpeakEntityID, getPlayerSprites(), getPlayerAnimations(), 1000);
}

static void setSpeakInactive() {
	removeBlitzEntity(gNPCHandler.mSpeakEntityID);
}

static void updateNPCs() {
	gNPCHandler.mCanSpeakPrev = gNPCHandler.mCanSpeak;
	gNPCHandler.mCanSpeak = 0;
	stl_int_map_map(gNPCHandler.mNPCs, updateSingleNPC);

	gNPCHandler.mCanSpeak = gNPCHandler.mCanSpeak && !gNPCHandler.mIsSpeaking;
	if (gNPCHandler.mCanSpeak != gNPCHandler.mCanSpeakPrev) {
		if (gNPCHandler.mCanSpeak) setSpeakActive();
		else setSpeakInactive();
	}
}

static void setTextBoxActive(string tName, string tText, int tIsExternal) {

	string nameText = tName + ":";
	gNPCHandler.mTextBoxAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 2000), getPlayerSprites(), makePosition(160, 220, 30));
	gNPCHandler.mNameTextID = addMugenTextMugenStyle(nameText.data(), makePosition(30, 163, 31), makeVector3DI(2, 0, 1));
	gNPCHandler.mTextID = addMugenTextMugenStyle(tText.data(), makePosition(30, 175, 31), makeVector3DI(3, 0, 1));
	setMugenTextBuildup(gNPCHandler.mTextID, 1);
	setMugenTextTextBoxWidth(gNPCHandler.mTextID, 250);

	gNPCHandler.mIsExternalText = tIsExternal;
	gNPCHandler.mIsSpeaking = 1;
}

static void setTextBoxActiveInternal() {

	NPC& e = gNPCHandler.mNPCs[gNPCHandler.mSpeakID];
	setPlayerInactive();
	setTextBoxActive(e.mName, e.mText[gNPCHandler.mCurrentTextStep], 0);
}

static void setTextBoxInactive() {
	removeMugenAnimation(gNPCHandler.mTextBoxAnimationID);
	removeMugenText(gNPCHandler.mNameTextID);
	removeMugenText(gNPCHandler.mTextID);
	gNPCHandler.mIsSpeaking = 0;
}

static void updateCanSpeak() {
	if (!gNPCHandler.mCanSpeak || gNPCHandler.mIsSpeaking || !getPlayerActive()) return;

	if (hasPressedAFlank()) {
		gNPCHandler.mCurrentTextStep = 0;
		setTextBoxActiveInternal();
	}
}

static void updateIsSpeaking() {
	if (!gNPCHandler.mIsSpeaking) return;

	if (hasPressedAFlank() || hasPressedStart()) {
		if (!isMugenTextBuiltUp(gNPCHandler.mTextID)) {
			setMugenTextBuiltUp(gNPCHandler.mTextID);
		}
		else {
			setTextBoxInactive();
			if (gNPCHandler.mIsExternalText) {
				gNPCHandler.mCB();
			}
			else {
				NPC& e = gNPCHandler.mNPCs[gNPCHandler.mSpeakID];
				setPlayerActive();
				if (gNPCHandler.mCurrentTextStep < e.mText.size() - 1) {
					gNPCHandler.mCurrentTextStep++;
					setTextBoxActiveInternal();
				}
			}
		}
	}
}

static void updateSpeak() {
	updateCanSpeak();
	updateIsSpeaking();
}

static void updateNPCHandler(void* tData) {
	updateNPCs();

	updateSpeak();
}

ActorBlueprint getNPCHandler()
{
	return makeActorBlueprint(loadNPCHandler, NULL, updateNPCHandler);
}

void loadNPCs(std::string tPath, MugenSpriteFile * tSprites, MugenAnimations * tAnimations)
{
	Buffer b = fileToBuffer(tPath.data());
	BufferPointer p = getBufferPointer(b);

	int amount = readIntegerFromTextStreamBufferPointer(&p);

	for (int i = 0; i < amount; i++) {
		NPC e;
		e.mID = readIntegerFromTextStreamBufferPointer(&p);
		int animation = readIntegerFromTextStreamBufferPointer(&p);

		e.mTile.x = readIntegerFromTextStreamBufferPointer(&p);
		e.mTile.y = readIntegerFromTextStreamBufferPointer(&p);

		e.mIsVisible = readIntegerFromTextStreamBufferPointer(&p);

		string flush = readLineFromTextStreamBufferPointer(&p);
		e.mName = readLineFromTextStreamBufferPointer(&p); 

		int lineAmount = readIntegerFromTextStreamBufferPointer(&p);
		flush = readLineFromTextStreamBufferPointer(&p);
		e.mText = vector<string>();
		for (int j = 0; j < lineAmount; j++) {
			string line = readLineFromTextStreamBufferPointer(&p);
			e.mText.push_back(line);		
		}

		e.mIsWalking = 0;

		Position pos = makePosition(e.mTile.x * 16 + 8, e.mTile.y * 16 + 8, 10 + ((e.mTile.y * 16 + 8) / (16 * 100.0)));
		e.mEntityID = addBlitzEntity(pos);
		addBlitzMugenAnimationComponent(e.mEntityID, tSprites, tAnimations, animation);
		setBlitzMugenAnimationVisibility(e.mEntityID, e.mIsVisible);
		gNPCHandler.mNPCs[e.mID] = e;
	}
	
	freeBuffer(b);
}

void setTextboxActiveExternal(string tName, string tTextbox, void(*tCB)())
{
	setTextBoxActive(tName, tTextbox, 1);
	gNPCHandler.mCB = tCB;
}

int hasTileNPC(Vector3DI tTarget)
{
	for (auto& e : gNPCHandler.mNPCs) {
		if (e.second.mIsVisible && e.second.mTile.x == tTarget.x && e.second.mTile.y == tTarget.y) return 1;
	}

	return 0;
}

void setNPCWalking(int tID, int tAnimation, Vector3DI tTarget, int tSpeed, void(*tCB)())
{
	NPC& e = gNPCHandler.mNPCs[tID];
	changeBlitzMugenAnimation(e.mEntityID, tAnimation);
	e.mCB = tCB;
	e.mSpeed = tSpeed;
	e.mTarget = tTarget;
	e.mIsWalking = 1;

}

void setNPCVisibility(int tID, int tVisibility)
{
	NPC& e = gNPCHandler.mNPCs[tID];
	setBlitzMugenAnimationVisibility(e.mEntityID, tVisibility);
	e.mIsVisible = tVisibility;
}

void setNPCAnimation(int tID, int tAnimation)
{
	NPC& e = gNPCHandler.mNPCs[tID];
	changeBlitzMugenAnimation(e.mEntityID, tAnimation);
}
