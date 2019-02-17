#include "fighthandler.h"

#include <utility>
#include <algorithm>

#include <prism/blitz.h>

#include "tilebg.h"
#include "overworldplayer.h"
#include "storyhandler.h"

typedef enum {
	FIGHT_SCREEN_SELECT_FIGHT,
	FIGHT_SCREEN_SELECT_ORDER,
	FIGHT_SCREEN_SELECT_ORDER_CHARACTER,
	FIGHT_SCREEN_SELECT_ORDER_FOR_CHARACTER,
	FIGHT_SCREEN_SELECT_ORDER_ENEMY,
	FIGHT_SCREEN_SELECT_ORDER_MAGIC,
	FIGHT_SCREEN_SELECT_ORDER_TARGET_CHARACTER,

} FightScreen;

typedef enum {
	FIGHT_ACTION_FIGHT,
	FIGHT_ACTION_MAGIC,
	FIGHT_ACTION_QUESTION,
	FIGHT_ACTION_DEFENSE,

} FightActionType;

typedef struct {
	int mTP;
	string tName;
} FightAttackDependency;

typedef struct {
	string mText;
	int mAnimation;
	int mDamage;
} FightAttack;

typedef struct {
	int mIsActive;

	int mBackAnimationID;
	int mBGID;
	int mNameTextID;
	int mHPNameTextID;
	int mHPValueTextID;
	int mMPNameTextID;
	int mMPValueTextID;
	int mActionAnimationID;

	int mHP;
	int mHPMax;
	int mTP;
	int mTPMax;
	int mSpeed;
	string mName;
	FightActionType mType;
	int mEnemyTarget;
	Position mBackPosition;

	int mSelectedMagic;

	FightAttack mDirectAttack;
	vector<pair<FightAttackDependency, FightAttack> > mMagicAttacks;

} FightCharacter;


typedef struct {
	int mHP;
	int mHPMax;
	int mSpeed;
	vector<FightAttack> mAttacks;
	Position mPosition;
	string mName;

	int mAnimationID;
} FightEnemy;

typedef struct {
	int mSelectText1;
	int mSelectText2;
	int mSelectBGID;
	int mFightBGID;
	int mStatusTextBGID;
	int mStatusTextID;

	int mMainSelection;
	int mOrderSelection;

	int mSelectorID;

	int mCharacterSelectorID;
	int mSelectedOrderCharacter;

	int mOrderBGID;
	int mOrderSelectorID;
	int mSelectedOrderForCharacter;

	int mEnemySelectorID;

	int mMagicBGID;
	int mMagicSelectorID;
	int mMagicName1;
	int mMagicCost1;
	int mMagicName2;
	int mMagicCost2;


} Select;

typedef struct {
	int mIsActive;

	vector<int> mFightOrder;
	int mCurrentFighter;

	FightAttack mActiveAttack;
	int mTarget;
	int mAttacksAll;

} FightSimulation;

static struct {
	int mIsActive;

	FightCharacter mCharacters[3];
	vector<FightEnemy> mEnemies;
	Select mSelect;
	FightSimulation mSimulation;

	FightScreen mActiveScreen;
	int mIsFightOver;

	int mIsFinalFight;

} gFightHandler;

static int getFightTypeAnimation(FightActionType tType) {
	if (tType == FIGHT_ACTION_FIGHT) return 3020;
	if (tType == FIGHT_ACTION_MAGIC) return 3021;
	if (tType == FIGHT_ACTION_QUESTION) return 3022;
	else return 3023;
}

static void loadSingleCharacter(int i, int tBackAnimation) {
	FightCharacter& e = gFightHandler.mCharacters[i];
	
	e.mBackAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), tBackAnimation), getPlayerSprites(), e.mBackPosition);
	setMugenAnimationInvisible(e.mBackAnimationID);

	e.mBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3004), getPlayerSprites(), makePosition(77 + i * 60, 181, 20));
	setMugenAnimationInvisible(e.mBGID);

	e.mNameTextID = addMugenTextMugenStyle("", makePosition(85 + i * 60, 198, 21), makeVector3DI(1, 0, 1));
	e.mHPNameTextID = addMugenTextMugenStyle("", makePosition(104 + i * 60, 207, 21), makeVector3DI(4, 0, 1));
	e.mHPValueTextID = addMugenTextMugenStyle("", makePosition(132 + i * 60, 213, 21), makeVector3DI(4, 0, -1));

	e.mMPNameTextID = addMugenTextMugenStyle("", makePosition(104 + i * 60, 219, 21), makeVector3DI(4, 0, 1));
	e.mMPValueTextID = addMugenTextMugenStyle("", makePosition(132 + i * 60, 225, 21), makeVector3DI(4, 0, -1));

	e.mActionAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), getFightTypeAnimation(e.mType)), getPlayerSprites(), makePosition(88 + i * 60, 206, 21));
	setMugenAnimationInvisible(e.mActionAnimationID);

	e.mIsActive = 1;
}

static void loadCharacters() {
	for (int i = 0; i < 3; i++) {
		loadSingleCharacter(i, 3000 + i);
	}
}

static void loadSelect() {

	gFightHandler.mSelect.mSelectText1 = addMugenTextMugenStyle("", makePosition(57, 209, 21), makeVector3DI(4, 0, 0));
	gFightHandler.mSelect.mSelectText2 = addMugenTextMugenStyle("", makePosition(57, 225, 21), makeVector3DI(4, 0, 0));

	gFightHandler.mSelect.mSelectBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3003), getPlayerSprites(), makePosition(32, 181, 20));
	setMugenAnimationInvisible(gFightHandler.mSelect.mSelectBGID);

	gFightHandler.mSelect.mFightBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3010), getPlayerSprites(), makePosition(0, 0, 15)); // TODO: load from level
	setMugenAnimationInvisible(gFightHandler.mSelect.mFightBGID);

	gFightHandler.mSelect.mStatusTextBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3005), getPlayerSprites(), makePosition(199, 149, 20)); 
	setMugenAnimationInvisible(gFightHandler.mSelect.mStatusTextBGID);
	gFightHandler.mSelect.mStatusTextID = addMugenTextMugenStyle("", makePosition(204, 161, 21), makeVector3DI(4, 0, 1));
	setMugenTextTextBoxWidth(gFightHandler.mSelect.mStatusTextID, 93);

	gFightHandler.mSelect.mSelectorID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3031), getPlayerSprites(), makePosition(0, 0, 21)); 
	setMugenAnimationInvisible(gFightHandler.mSelect.mSelectorID);

	gFightHandler.mSelect.mCharacterSelectorID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3032), getPlayerSprites(), makePosition(0, 0, 21));
	setMugenAnimationInvisible(gFightHandler.mSelect.mCharacterSelectorID);

	gFightHandler.mSelect.mOrderBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3040), getPlayerSprites(), makePosition(40, 149, 20));
	setMugenAnimationInvisible(gFightHandler.mSelect.mOrderBGID);

	gFightHandler.mSelect.mOrderSelectorID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3032), getPlayerSprites(), makePosition(0, 0, 21));
	setMugenAnimationInvisible(gFightHandler.mSelect.mOrderSelectorID);

	gFightHandler.mSelect.mEnemySelectorID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3032), getPlayerSprites(), makePosition(0, 0, 21));
	setMugenAnimationInvisible(gFightHandler.mSelect.mEnemySelectorID);

	gFightHandler.mSelect.mMagicBGID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3006), getPlayerSprites(), makePosition(137, 135, 20));
	setMugenAnimationInvisible(gFightHandler.mSelect.mMagicBGID);

	gFightHandler.mSelect.mMagicSelectorID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 3032), getPlayerSprites(), makePosition(0, 0, 21));
	setMugenAnimationInvisible(gFightHandler.mSelect.mMagicSelectorID);

	gFightHandler.mSelect.mMagicName1 = addMugenTextMugenStyle("", makePosition(146, 151, 21), makeVector3DI(4, 0, 1));
	gFightHandler.mSelect.mMagicCost1 = addMugenTextMugenStyle("", makePosition(146, 158, 21), makeVector3DI(4, 0, 1));

	gFightHandler.mSelect.mMagicName2 = addMugenTextMugenStyle("", makePosition(146, 171, 21), makeVector3DI(4, 0, 1));
	gFightHandler.mSelect.mMagicCost2 = addMugenTextMugenStyle("", makePosition(146, 178, 21), makeVector3DI(4, 0, 1));
}

static string parseAttackString(string tText, int& tAttacksAll, string tTargetName) {
	
	tAttacksAll = 1;
	string out;
	for (int i = 0; i < tText.size(); i++) {
		if (tText[i] == '$') {
			out += tTargetName;
			tAttacksAll = 0;
		}
		else {
			out += tText[i];
		}
	}
	return out;
}

static void setCharacterVisible(int i) {
	FightCharacter& e = gFightHandler.mCharacters[i];

	setMugenAnimationPosition(e.mBackAnimationID, makePosition(160, 181, 18));
	setMugenAnimationVisibility(e.mBackAnimationID, 1);
}

static void setCharactersVisible() {
	for (int i = 0; i < 3; i++) {
		FightCharacter& e = gFightHandler.mCharacters[i];
		if (!e.mIsActive) continue;
		if (!e.mHP) continue;

		setMugenAnimationPosition(e.mBackAnimationID, e.mBackPosition);
		setMugenAnimationVisibility(e.mBackAnimationID, 1);
	}
}

static void setCharactersInvisible() {
	for (int i = 0; i < 3; i++) {
		FightCharacter& e = gFightHandler.mCharacters[i];
		if (!e.mIsActive) continue;

		setMugenAnimationVisibility(e.mBackAnimationID, 0);
	}
}

static void setEnemyVisible(int i) {
	FightEnemy& e = gFightHandler.mEnemies[i];


	setMugenAnimationPosition(e.mAnimationID, makePosition(160, 100, 18));
	setMugenAnimationVisibility(e.mAnimationID, 1);
}

static void setEnemiesVisible() {
	for (int i = 0; i < gFightHandler.mEnemies.size(); i++) {
		FightEnemy& e = gFightHandler.mEnemies[i];
		if (!e.mHP) continue;

		setMugenAnimationPosition(e.mAnimationID, e.mPosition);
		setMugenAnimationVisibility(e.mAnimationID, 1);
	}
}

static void setEnemiesInvisible() {
	for (int i = 0; i < gFightHandler.mEnemies.size(); i++) {
		FightEnemy& e = gFightHandler.mEnemies[i];

		setMugenAnimationVisibility(e.mAnimationID, 0);
	}
}

static void simulateNextFightStep();

static void setFightSimulationOver() {
	setCharactersVisible();
	setEnemiesVisible();

	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3031));
	gFightHandler.mSimulation.mIsActive = 0;
}

static void gotoNextFightStep() {
	setCharactersInvisible();
	setEnemiesInvisible();

	gFightHandler.mSimulation.mCurrentFighter++;
	if (gFightHandler.mSimulation.mCurrentFighter < gFightHandler.mSimulation.mFightOrder.size()) {
		simulateNextFightStep();
	}
	else {
		setFightSimulationOver();
	}
}

static void setStatusTextVisible(string tText) {
	setMugenAnimationVisibility(gFightHandler.mSelect.mStatusTextBGID, 1);
	changeMugenText(gFightHandler.mSelect.mStatusTextID, tText.data());

}

static void setStatusTextInvisible() {
	setMugenAnimationVisibility(gFightHandler.mSelect.mStatusTextBGID, 0);
	changeMugenText(gFightHandler.mSelect.mStatusTextID, "");
}

static void enemyAttackPauseFinished(void* tCaller) {
	(void)tCaller;
	setStatusTextInvisible();
	gotoNextFightStep();
}

static void updateHealthAndTPText(FightCharacter& e) {
	char text[200];
	changeMugenText(e.mHPNameTextID, "HP:");
	sprintf(text, "%d/%d", e.mHP, e.mHPMax);
	changeMugenText(e.mHPValueTextID, text);

	changeMugenText(e.mMPNameTextID, "MP:");
	sprintf(text, "%d/%d", e.mTP, e.mTPMax);
	changeMugenText(e.mMPValueTextID, text);
}

static void fightFinished(void* tCaller);

static void decreaseCharacterHP(int i, int tDamage) {
	if (gFightHandler.mIsFightOver) return;
	FightCharacter& e = gFightHandler.mCharacters[i];

	if (e.mType == FIGHT_ACTION_DEFENSE) {
		tDamage /= 2;
	}

	e.mHP = max(0, e.mHP - tDamage);
	updateHealthAndTPText(e);

	int isOver = 1;
	for (int j = 0; j < 3; j++) {
		if (gFightHandler.mCharacters[j].mIsActive && gFightHandler.mCharacters[j].mHP) {
			isOver = 0;
		}
	}

	if (isOver) {
		setStatusTextVisible("THE PARTY LOST!");
		setEnemiesVisible();
		addTimerCB(180, fightFinished, NULL);
		restorePlayerAfterDeath();
		gFightHandler.mIsFightOver = 1;
	}
}

static void enemyAttackAnimationFinished(void* tCaller) {
	if (gFightHandler.mSimulation.mAttacksAll) {
		for (int i = 0; i < 3; i++) {
			FightCharacter& e = gFightHandler.mCharacters[i];
			if (!e.mIsActive || !e.mHP) continue;
			decreaseCharacterHP(i, gFightHandler.mSimulation.mActiveAttack.mDamage);
		} 
	}
	else {
		decreaseCharacterHP(gFightHandler.mSimulation.mTarget, gFightHandler.mSimulation.mActiveAttack.mDamage);
	}

	if (gFightHandler.mIsFightOver) return;

	addTimerCB(120, enemyAttackPauseFinished, NULL);
}

static void simulateEnemyFightStep(int tEnemyID) {
	FightEnemy& e = gFightHandler.mEnemies[tEnemyID];
	if (!e.mHP) {
		gotoNextFightStep();
		return;
	}

	FightAttack& attack = e.mAttacks[randfromInteger(0, e.mAttacks.size() - 1)];

	gFightHandler.mSimulation.mTarget = 0;
	for (int i = 0; i < 100; i++) {
		gFightHandler.mSimulation.mTarget = randfromInteger(0, 2);
		if (gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget].mIsActive && gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget].mHP) break;
	}
	string targetName = gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget].mName;
	string attackText = parseAttackString(attack.mText, gFightHandler.mSimulation.mAttacksAll, targetName);
	gFightHandler.mSimulation.mActiveAttack = attack;
	Position animationPos = makePosition(160, 160, 22);
	if (gFightHandler.mSimulation.mAttacksAll) {
		setCharactersVisible();
	}
	else {
		setCharacterVisible(gFightHandler.mSimulation.mTarget);
	}
	setEnemyVisible(tEnemyID);

	setStatusTextVisible(attackText);

	int attackAnimationID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), attack.mAnimation), getLevelSprites(), animationPos);
	setMugenAnimationNoLoop(attackAnimationID);
	setMugenAnimationCallback(attackAnimationID, enemyAttackAnimationFinished, NULL);
}

static void characterAttackPauseFinished(void* tCaller) {
	(void)tCaller;
	setStatusTextInvisible();
	gotoNextFightStep();
}

static void setFightInactive();

static void fightFinished(void* tCaller) {
	(void)tCaller;
	setFightInactive();
}

static void decreaseEnemyHP(int i, int tDamage) {
	FightEnemy& e = gFightHandler.mEnemies[i];

	e.mHP = max(0, e.mHP - tDamage);

	int isOver = 1;
	for (int j = 0; j < gFightHandler.mEnemies.size(); j++) {
		if (gFightHandler.mEnemies[j].mHP) {
			isOver = 0;
		}
	}

	if (isOver) {
		setStatusTextVisible("OSCR AND THE RIFFRAFF WON!");
		setEnemiesInvisible();
		if (gFightHandler.mIsFinalFight) {
			increaseStoryPart();
		}
		addTimerCB(180, fightFinished, NULL);
		gFightHandler.mIsFightOver = 1;
	}
}

static void characterAttackAnimationFinished(void* tCaller) {
	if (gFightHandler.mSimulation.mAttacksAll) {
		for (int i = 0; i < gFightHandler.mEnemies.size(); i++) {
			FightEnemy& e = gFightHandler.mEnemies[i];
			if (!e.mHP) continue;
			decreaseEnemyHP(i, gFightHandler.mSimulation.mActiveAttack.mDamage);
		}
	}
	else {
		decreaseEnemyHP(gFightHandler.mSimulation.mTarget, gFightHandler.mSimulation.mActiveAttack.mDamage);
	}

	if (gFightHandler.mIsFightOver) return;

	addTimerCB(120, characterAttackPauseFinished, NULL);
}

static void characterHealAnimationFinished(void* tCaller) {
	if (gFightHandler.mSimulation.mAttacksAll) {
		for (int i = 0; i < 3; i++) {
			FightCharacter& e = gFightHandler.mCharacters[i];
			if (!e.mHP) continue;
			if (!e.mIsActive) continue;
			e.mHP = min(e.mHP - gFightHandler.mSimulation.mActiveAttack.mDamage, e.mHPMax);
		}
	}
	else {
		FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget];
		e.mHP = min(e.mHP - gFightHandler.mSimulation.mActiveAttack.mDamage, e.mHPMax);
	}

	for (int i = 0; i < 3; i++) updateHealthAndTPText(gFightHandler.mCharacters[i]);

	addTimerCB(120, characterAttackPauseFinished, NULL);
}

static void fixEnemyTarget(FightCharacter& e) {
	if (e.mEnemyTarget >= gFightHandler.mEnemies.size() || !gFightHandler.mEnemies[e.mEnemyTarget].mHP) {
		for (int i = 0; i < 100; i++) {
			gFightHandler.mSimulation.mTarget = randfromInteger(0, gFightHandler.mEnemies.size() - 1);
			if (gFightHandler.mEnemies[gFightHandler.mSimulation.mTarget].mHP) break;
		}
		e.mEnemyTarget = gFightHandler.mSimulation.mTarget;
	}

	gFightHandler.mSimulation.mTarget = e.mEnemyTarget;
}

static void fixPlayerTarget(FightCharacter& e) {
	if (e.mEnemyTarget >= 3 || !gFightHandler.mCharacters[e.mEnemyTarget].mHP || !gFightHandler.mCharacters[e.mEnemyTarget].mIsActive) {
		for (int i = 0; i < 100; i++) {
			gFightHandler.mSimulation.mTarget = randfromInteger(0, 2);
			if (gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget].mHP && gFightHandler.mCharacters[gFightHandler.mSimulation.mTarget].mIsActive) break;
		}
		e.mEnemyTarget = gFightHandler.mSimulation.mTarget;
	}

	gFightHandler.mSimulation.mTarget = e.mEnemyTarget;
}


static void checkoutPauseFinished(void* tCaller) {
	(void)tCaller;
	int id = gFightHandler.mSimulation.mFightOrder[gFightHandler.mSimulation.mCurrentFighter];
	FightCharacter& e = gFightHandler.mCharacters[id];
	FightEnemy enemy = gFightHandler.mEnemies[e.mEnemyTarget];
	char text[200];
	sprintf(text, "%s has %d/%d HP!", enemy.mName.data(), enemy.mHP, enemy.mHPMax);
	setStatusTextVisible(text);
	addTimerCB(120, characterAttackPauseFinished, NULL);
}

static void simulateCharacterFightStep(int tID) {
	FightCharacter& e = gFightHandler.mCharacters[tID];
	if (!e.mHP) {
		gotoNextFightStep();
		return;
	}




	if (e.mType == FIGHT_ACTION_FIGHT) {
		fixEnemyTarget(e);
		string targetName = gFightHandler.mEnemies[e.mEnemyTarget].mName;
		setCharacterVisible(tID);
		setEnemyVisible(e.mEnemyTarget);
		gFightHandler.mSimulation.mActiveAttack = e.mDirectAttack;
		string attackText = parseAttackString(gFightHandler.mSimulation.mActiveAttack.mText, gFightHandler.mSimulation.mAttacksAll, targetName);
		setStatusTextVisible(attackText);
		int attackAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), gFightHandler.mSimulation.mActiveAttack.mAnimation), getPlayerSprites(), makePosition(160, 60, 22));
		setMugenAnimationNoLoop(attackAnimationID);
		setMugenAnimationCallback(attackAnimationID, characterAttackAnimationFinished, NULL);
	}
	else if (e.mType == FIGHT_ACTION_MAGIC) {
		FightAttackDependency& dependency = e.mMagicAttacks[e.mSelectedMagic].first;
		FightAttack& magic = e.mMagicAttacks[e.mSelectedMagic].second;

		gFightHandler.mSimulation.mActiveAttack = magic;

		if (e.mTP < dependency.mTP) {
			setCharacterVisible(tID);
			string statusText = e.mName + " tries to use " + dependency.tName + ", but does not have enough MP!";
			setStatusTextVisible(statusText);
			addTimerCB(120, characterAttackPauseFinished, NULL);
			return;
		}

		e.mTP -= dependency.mTP;
		updateHealthAndTPText(e);

		if (magic.mDamage < 0) {
			fixPlayerTarget(e);
			setEnemiesInvisible();
			setCharactersVisible();
			string targetName = gFightHandler.mCharacters[e.mEnemyTarget].mName;
			string attackText = parseAttackString(gFightHandler.mSimulation.mActiveAttack.mText, gFightHandler.mSimulation.mAttacksAll, targetName);
			setStatusTextVisible(attackText);
			int attackAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), gFightHandler.mSimulation.mActiveAttack.mAnimation), getPlayerSprites(), makePosition(160, 160, 22));
			setMugenAnimationNoLoop(attackAnimationID);
			setMugenAnimationCallback(attackAnimationID, characterHealAnimationFinished, NULL);
		}
		else {
			fixEnemyTarget(e);
			setCharacterVisible(tID);
			if (gFightHandler.mSimulation.mAttacksAll) setEnemiesVisible();
			else setEnemyVisible(e.mEnemyTarget);
			string targetName = gFightHandler.mEnemies[e.mEnemyTarget].mName;
			string attackText = parseAttackString(gFightHandler.mSimulation.mActiveAttack.mText, gFightHandler.mSimulation.mAttacksAll, targetName);
			setStatusTextVisible(attackText);
			int attackAnimationID = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), gFightHandler.mSimulation.mActiveAttack.mAnimation), getPlayerSprites(), makePosition(160, 60, 22));
			setMugenAnimationNoLoop(attackAnimationID);
			setMugenAnimationCallback(attackAnimationID, characterAttackAnimationFinished, NULL);
		}
	}
	else if (e.mType == FIGHT_ACTION_QUESTION) {
		fixEnemyTarget(e);
		FightEnemy enemy = gFightHandler.mEnemies[e.mEnemyTarget];
		setCharacterVisible(tID);
		setEnemyVisible(e.mEnemyTarget);
		gFightHandler.mSimulation.mActiveAttack = e.mDirectAttack;
		string attackText = e.mName + " checks out " + enemy.mName + "!";
		setStatusTextVisible(attackText);
		addTimerCB(120, checkoutPauseFinished, NULL);
	}
	else {
		setCharacterVisible(tID);
		setStatusTextVisible(e.mName + " defends! 10 MP restored!");
		e.mTP = min(e.mTP + 10, e.mTPMax);
		updateHealthAndTPText(e);
		addTimerCB(120, characterAttackPauseFinished, NULL);
	}
}

static void simulateNextFightStep() {
	int id = gFightHandler.mSimulation.mFightOrder[gFightHandler.mSimulation.mCurrentFighter];

	setCharactersInvisible();
	setEnemiesInvisible();

	if (id < 0) {
		simulateEnemyFightStep((-id) - 1);
	}
	else {
		simulateCharacterFightStep(id);
	}

}

static void calculateFightOrder() {
	vector<pair<int, int> > fighterList;

	for(int i = 0; i < 3; i++) {
		FightCharacter& e = gFightHandler.mCharacters[i];
		if (!e.mIsActive) continue;

		fighterList.push_back(make_pair(-e.mSpeed, i));
	}

	for (int i = 0; i < gFightHandler.mEnemies.size(); i++) {
		FightEnemy& e = gFightHandler.mEnemies[i];
		if (!e.mHP) continue;
		fighterList.push_back(make_pair(-e.mSpeed, -(i + 1)));
	}

	sort(fighterList.begin(), fighterList.end());

	gFightHandler.mSimulation.mFightOrder.clear();
	for (int i = 0; i < fighterList.size(); i++) {
		gFightHandler.mSimulation.mFightOrder.push_back(fighterList[i].second);
	}
}

static void calculateFightOrderOnlyEnemies() {
	vector<pair<int, int> > fighterList;

	for (int i = 0; i < gFightHandler.mEnemies.size(); i++) {
		FightEnemy& e = gFightHandler.mEnemies[i];
		if (!e.mHP) continue;
		fighterList.push_back(make_pair(-e.mSpeed, -(i + 1)));
	}

	sort(fighterList.begin(), fighterList.end());

	gFightHandler.mSimulation.mFightOrder.clear();
	for (int i = 0; i < fighterList.size(); i++) {
		gFightHandler.mSimulation.mFightOrder.push_back(fighterList[i].second);
	}
}

static void setFightSimulationOverAfterFightOrder() {
	gFightHandler.mSimulation.mCurrentFighter = 0;

	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3030));
	gFightHandler.mSimulation.mIsActive = 1;

	simulateNextFightStep();
}

static void setFightSimulationActive() {

	calculateFightOrder();
	setFightSimulationOverAfterFightOrder();
}

static void setFightSimulationOnlyEnemies() {

	calculateFightOrderOnlyEnemies();
	setFightSimulationOverAfterFightOrder();
}

static void setSelectUIFightSelect() {
	changeMugenText(gFightHandler.mSelect.mSelectText1, "FGHT");
	changeMugenText(gFightHandler.mSelect.mSelectText2, "STGY");

	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3031));
	setMugenAnimationPosition(gFightHandler.mSelect.mSelectorID, makePosition(50, !gFightHandler.mSelect.mMainSelection ? 194 : 210, 21));
	setMugenAnimationVisibility(gFightHandler.mSelect.mSelectorID, 1);

	gFightHandler.mActiveScreen = FIGHT_SCREEN_SELECT_FIGHT;
}

static void unsetSelectOrderMagic() {
	setMugenAnimationVisibility(gFightHandler.mSelect.mMagicBGID, 0);
	setMugenAnimationVisibility(gFightHandler.mSelect.mMagicSelectorID, 0);

	changeMugenText(gFightHandler.mSelect.mMagicName1, "");
	changeMugenText(gFightHandler.mSelect.mMagicCost1, "");

	changeMugenText(gFightHandler.mSelect.mMagicName2, "");
	changeMugenText(gFightHandler.mSelect.mMagicCost2, "");
}

static void setSelectUIOrderSelect() {
	changeMugenText(gFightHandler.mSelect.mSelectText1, "ORDR");
	changeMugenText(gFightHandler.mSelect.mSelectText2, "RUN");

	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3031));
	setMugenAnimationPosition(gFightHandler.mSelect.mSelectorID, makePosition(50, !gFightHandler.mSelect.mOrderSelection ? 194 : 210, 21));
	setMugenAnimationVisibility(gFightHandler.mSelect.mSelectorID, 1);

	setMugenAnimationVisibility(gFightHandler.mSelect.mCharacterSelectorID, 0);

	unsetSelectOrderMagic();
	setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 0);
	setMugenAnimationVisibility(gFightHandler.mSelect.mOrderBGID, 0);
	setMugenAnimationVisibility(gFightHandler.mSelect.mOrderSelectorID, 0);

	gFightHandler.mActiveScreen = FIGHT_SCREEN_SELECT_ORDER;
}

static void updateMainSelect() {
	if (hasPressedUpFlank() || hasPressedDownFlank()) {
		gFightHandler.mSelect.mMainSelection ^= 1;
		setMugenAnimationPosition(gFightHandler.mSelect.mSelectorID, makePosition(50, !gFightHandler.mSelect.mMainSelection ? 194 : 210, 21));
	}

	if (hasPressedAFlank()) {
		if (!gFightHandler.mSelect.mMainSelection) {
			setFightSimulationActive();
		}
		else {
			setSelectUIOrderSelect();
		}
	}
}

static void runAwayFinishedSuccessful(void* tCaller) {
	(void)tCaller;
	setFightInactive();
}

static void runAwayFinishedFailed(void* tCaller) {
	(void)tCaller;
	setFightSimulationOnlyEnemies();
}

static void tryRunAway() {
	setCharactersVisible();

	gFightHandler.mSelect.mMainSelection = 0;
	setSelectUIFightSelect();

	gFightHandler.mSimulation.mIsActive = 1;
	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3030));

	double prob = 1;
	double attempt = randfrom(0, 1);

	if (attempt < prob) {
		setEnemiesInvisible();
		setStatusTextVisible("The group manages to get away! The enemy group is left heartbroken!");
		gFightHandler.mIsFightOver = 1;
		addTimerCB(120, runAwayFinishedSuccessful, NULL);
	}
	else {
		setEnemiesVisible();
		setStatusTextVisible("The group tries to get away but feels awkward just running away and decides to stay for a little longer!");
		addTimerCB(120, runAwayFinishedFailed, NULL);
	}
}

static void updateSelectedMagicPosition() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];
	setMugenAnimationPosition(gFightHandler.mSelect.mMagicSelectorID, makePosition(155, 160 + e.mSelectedMagic * 20, 22));
}

static void setSelectOrderMagic() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];

	changeMugenAnimation(gFightHandler.mSelect.mOrderSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));

	setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 0);

	setMugenAnimationVisibility(gFightHandler.mSelect.mMagicBGID, 1);
	setMugenAnimationVisibility(gFightHandler.mSelect.mMagicSelectorID, 1);
	changeMugenAnimation(gFightHandler.mSelect.mMagicSelectorID, getMugenAnimation(getPlayerAnimations(), 3032));

	char text[100];
	changeMugenText(gFightHandler.mSelect.mMagicName1, e.mMagicAttacks[0].first.tName.data());
	sprintf(text, "Cost: %dTP", e.mMagicAttacks[0].first.mTP);
	changeMugenText(gFightHandler.mSelect.mMagicCost1, text);

	changeMugenText(gFightHandler.mSelect.mMagicName2, e.mMagicAttacks[1].first.tName.data());
	sprintf(text, "Cost: %dTP", e.mMagicAttacks[1].first.mTP);
	changeMugenText(gFightHandler.mSelect.mMagicCost2, text);
	updateSelectedMagicPosition();

	gFightHandler.mActiveScreen = FIGHT_SCREEN_SELECT_ORDER_MAGIC;
}

static void updateSelectOrderEnemyPosition() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];
	fixEnemyTarget(e);
	FightEnemy& enemy = gFightHandler.mEnemies[e.mEnemyTarget];
	Position enemyPos = getMugenAnimationPosition(enemy.mAnimationID);

	setMugenAnimationPosition(gFightHandler.mSelect.mEnemySelectorID, enemyPos + makePosition(0, 5, 1));
	setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 1);
}

static void changeSelectOrderEnemy(int tDelta) {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];
	
	e.mEnemyTarget = (e.mEnemyTarget + tDelta + gFightHandler.mEnemies.size()) % gFightHandler.mEnemies.size();
	for (int i = 0; i < 100; i++) {
		if (gFightHandler.mEnemies[e.mEnemyTarget].mHP) break;
		e.mEnemyTarget = (e.mEnemyTarget + tDelta + gFightHandler.mEnemies.size()) % gFightHandler.mEnemies.size();
	}

	updateSelectOrderEnemyPosition();
}

static void setSelectOrderEnemy() {
	updateSelectOrderEnemyPosition();

	setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 1);

	if (gFightHandler.mSelect.mSelectedOrderForCharacter == 1) {
		changeMugenAnimation(gFightHandler.mSelect.mMagicSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));
	}

	changeMugenAnimation(gFightHandler.mSelect.mOrderSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));
	gFightHandler.mActiveScreen = FIGHT_SCREEN_SELECT_ORDER_ENEMY;
}

static void updateSelectOrderForCharacterPosition() {
	setMugenAnimationPosition(gFightHandler.mSelect.mOrderSelectorID, makePosition(61 + gFightHandler.mSelect.mSelectedOrderForCharacter * 19, 175, 22));
	setMugenAnimationVisibility(gFightHandler.mSelect.mOrderSelectorID, 1);
}

static void setSelectOrderForCharacter() {
	updateSelectOrderForCharacterPosition();

	setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 0);

	setMugenAnimationVisibility(gFightHandler.mSelect.mOrderBGID, 1);
	setMugenAnimationVisibility(gFightHandler.mSelect.mOrderSelectorID, 1);

	changeMugenAnimation(gFightHandler.mSelect.mOrderSelectorID, getMugenAnimation(getPlayerAnimations(), 3032));
	changeMugenAnimation(gFightHandler.mSelect.mCharacterSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));
	gFightHandler.mActiveScreen = FIGHT_SCREEN_SELECT_ORDER_FOR_CHARACTER;
}

static void fixSelectOrderCharacter(int& tIndex) {
	for (int i = 0; i < 100; i++) {
		if (gFightHandler.mCharacters[tIndex].mIsActive && gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mHP) break;
		tIndex = (tIndex + 1) % 3;
	}
}

static void updateSelectCharacterOrderPosition(int tIndex) {
	setMugenAnimationPosition(gFightHandler.mSelect.mCharacterSelectorID, makePosition(94 + tIndex * 60, 223, 22));
	setMugenAnimationVisibility(gFightHandler.mSelect.mCharacterSelectorID, 1);
}

static void setSelectOrderCharacter(FightScreen tScreen) {
	
	if (tScreen == FIGHT_SCREEN_SELECT_ORDER_CHARACTER) {
		fixSelectOrderCharacter(gFightHandler.mSelect.mSelectedOrderCharacter);
		updateSelectCharacterOrderPosition(gFightHandler.mSelect.mSelectedOrderCharacter);
		setMugenAnimationVisibility(gFightHandler.mSelect.mEnemySelectorID, 0);
		setMugenAnimationVisibility(gFightHandler.mSelect.mOrderBGID, 0);
		setMugenAnimationVisibility(gFightHandler.mSelect.mOrderSelectorID, 0);
	}
	else {
		FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];
		e.mEnemyTarget = gFightHandler.mSelect.mSelectedOrderCharacter;
		fixSelectOrderCharacter(e.mEnemyTarget);
		updateSelectCharacterOrderPosition(e.mEnemyTarget);

		changeMugenAnimation(gFightHandler.mSelect.mMagicSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));
	}

	changeMugenAnimation(gFightHandler.mSelect.mCharacterSelectorID, getMugenAnimation(getPlayerAnimations(), 3032));

	changeMugenAnimation(gFightHandler.mSelect.mSelectorID, getMugenAnimation(getPlayerAnimations(), 3030));
	gFightHandler.mActiveScreen = tScreen;
}



static void updateOrderSelect() {
	if (hasPressedUpFlank() || hasPressedDownFlank()) {
		gFightHandler.mSelect.mOrderSelection ^= 1;
		setMugenAnimationPosition(gFightHandler.mSelect.mSelectorID, makePosition(50, !gFightHandler.mSelect.mOrderSelection ? 194 : 210, 21));
	}

	if (hasPressedAFlank()) {
		if (!gFightHandler.mSelect.mOrderSelection) {
			setSelectOrderCharacter(FIGHT_SCREEN_SELECT_ORDER_CHARACTER);
		}
		else {
			tryRunAway();
		}
	}

	if (hasPressedBFlank()) {
		setSelectUIFightSelect();
	}
}

static void updateOrderSelectCharacter() {
	if (hasPressedRightFlank()) {
		gFightHandler.mSelect.mSelectedOrderCharacter = (gFightHandler.mSelect.mSelectedOrderCharacter + 1) % 3;		
		fixSelectOrderCharacter(gFightHandler.mSelect.mSelectedOrderCharacter);
		updateSelectCharacterOrderPosition(gFightHandler.mSelect.mSelectedOrderCharacter);
	}
	else if (hasPressedLeftFlank()) {
		gFightHandler.mSelect.mSelectedOrderCharacter = (gFightHandler.mSelect.mSelectedOrderCharacter - 1 + 3) % 3;
		for (int i = 0; i < 100; i++) {
			if (gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mIsActive && gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mHP) break;
			gFightHandler.mSelect.mSelectedOrderCharacter = (gFightHandler.mSelect.mSelectedOrderCharacter - 1 + 3) % 3;
		}
		updateSelectCharacterOrderPosition(gFightHandler.mSelect.mSelectedOrderCharacter);
	}
	
	if (hasPressedAFlank()) {
		setSelectOrderForCharacter();
	}

	if (hasPressedBFlank()) {
		setSelectUIOrderSelect();
	}
}

static void updateCharacterIcon() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];
	changeMugenAnimation(e.mActionAnimationID, getMugenAnimation(getPlayerAnimations(), getFightTypeAnimation(e.mType)));
}

static void setCharacterMagic() {
	gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mType = FIGHT_ACTION_MAGIC;
	updateCharacterIcon();
}

static void setCharacterDefense() {
	gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mType = FIGHT_ACTION_DEFENSE;
	updateCharacterIcon();
}

static void setCharacterFight() {
	gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mType = FIGHT_ACTION_FIGHT;
	updateCharacterIcon();
}

static void setCharacterQuestion() {
	gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter].mType = FIGHT_ACTION_QUESTION;
	updateCharacterIcon();
}

static void updateOrderSelectForCharacter() {
	if (hasPressedRightFlank()) {
		gFightHandler.mSelect.mSelectedOrderForCharacter = (gFightHandler.mSelect.mSelectedOrderForCharacter + 1) % 4;
		updateSelectOrderForCharacterPosition();
	}
	else if (hasPressedLeftFlank()) {
		gFightHandler.mSelect.mSelectedOrderForCharacter = (gFightHandler.mSelect.mSelectedOrderForCharacter - 1 + 4) % 4;
		updateSelectOrderForCharacterPosition();
	}

	if (hasPressedAFlank()) {
		if (gFightHandler.mSelect.mSelectedOrderForCharacter == 0) {
			setSelectOrderEnemy();
		} else if (gFightHandler.mSelect.mSelectedOrderForCharacter == 1) {
			setSelectOrderMagic();
		}
		else if (gFightHandler.mSelect.mSelectedOrderForCharacter == 2) {
			setSelectOrderEnemy();
		}
		else if (gFightHandler.mSelect.mSelectedOrderForCharacter == 3) {
			setCharacterDefense();
			setSelectUIOrderSelect();
		}
	}

	if (hasPressedBFlank()) {
		setSelectOrderCharacter(FIGHT_SCREEN_SELECT_ORDER_CHARACTER);
	}
}

static void updateOrderSelectEnemy() {
	if (hasPressedRightFlank()) {
		changeSelectOrderEnemy(1);
	}
	else if (hasPressedLeftFlank()) {
		changeSelectOrderEnemy(-1);
	}

	if (hasPressedAFlank()) {
		if (gFightHandler.mSelect.mSelectedOrderForCharacter == 0) {
			setCharacterFight();
			setSelectUIOrderSelect();
		} else if (gFightHandler.mSelect.mSelectedOrderForCharacter == 1) {
			setCharacterMagic();
			setSelectUIOrderSelect();
		}
		else 	if (gFightHandler.mSelect.mSelectedOrderForCharacter == 2) {
			setCharacterQuestion();
			setSelectUIOrderSelect();
		}
	}

	if (hasPressedBFlank()) {
		if (gFightHandler.mSelect.mSelectedOrderForCharacter == 1) {
			setSelectOrderMagic();
		}
		else {
			setSelectOrderForCharacter();
		}
	}
}
static int doesHitAll(FightAttack& tAttack) {
	for (int i = 0; i < tAttack.mText.size(); i++) {
		if (tAttack.mText[i] == '$') return 0;
	}

	return 1;
}

static void updateOrderSelectMagic() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];

	if (hasPressedUpFlank() || hasPressedDownFlank()) {
		e.mSelectedMagic ^= 1;
		updateSelectedMagicPosition();
	}

	if (hasPressedAFlank()) {
		FightAttack& magic = e.mMagicAttacks[e.mSelectedMagic].second;
		if (doesHitAll(magic)) {
			setCharacterMagic();
			setSelectUIOrderSelect();
		}
		else if (magic.mDamage < 0) {
			setSelectOrderCharacter(FIGHT_SCREEN_SELECT_ORDER_TARGET_CHARACTER);
		}
		else {
			setSelectOrderEnemy();
		}


	}

	if (hasPressedBFlank()) {
		unsetSelectOrderMagic();
		setSelectOrderForCharacter();
	}
}

static void resetCharacterSelect() {
	updateSelectCharacterOrderPosition(gFightHandler.mSelect.mSelectedOrderCharacter);
	changeMugenAnimation(gFightHandler.mSelect.mCharacterSelectorID, getMugenAnimation(getPlayerAnimations(), 3033));
}

static void updateOrderSelectCharacterTarget() {
	FightCharacter& e = gFightHandler.mCharacters[gFightHandler.mSelect.mSelectedOrderCharacter];

	if (hasPressedRightFlank()) {
		e.mEnemyTarget = (e.mEnemyTarget + 1) % 3;
		fixSelectOrderCharacter(e.mEnemyTarget);
		updateSelectCharacterOrderPosition(e.mEnemyTarget);
	}
	else if (hasPressedLeftFlank()) {
		e.mEnemyTarget = (e.mEnemyTarget - 1 + 3) % 3;
		for (int i = 0; i < 100; i++) {
			if (gFightHandler.mCharacters[e.mEnemyTarget].mIsActive && gFightHandler.mCharacters[e.mEnemyTarget].mHP) break;
			e.mEnemyTarget = (e.mEnemyTarget - 1 + 3) % 3;
		}
		updateSelectCharacterOrderPosition(e.mEnemyTarget);
	}

	if (hasPressedAFlank()) {
		setCharacterMagic();
		setSelectUIOrderSelect();
	}

	if (hasPressedBFlank()) {
		resetCharacterSelect();
		setSelectOrderMagic();
	}
}

static void updateFightHandler(void* tData) {
	(void)tData;
	if (!gFightHandler.mIsActive) return;
	if (gFightHandler.mSimulation.mIsActive) return;
	if (gFightHandler.mIsFightOver) return;

	if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_FIGHT) {
		updateMainSelect();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER) {
		updateOrderSelect();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER_CHARACTER) {
		updateOrderSelectCharacter();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER_FOR_CHARACTER) {
		updateOrderSelectForCharacter();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER_ENEMY) {
		updateOrderSelectEnemy();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER_MAGIC) {
		updateOrderSelectMagic();
	}
	else if (gFightHandler.mActiveScreen == FIGHT_SCREEN_SELECT_ORDER_TARGET_CHARACTER) {
		updateOrderSelectCharacterTarget();
	}

}

static void loadFightHandler(void* tData) {
	(void)tData;

	loadCharacters();
	loadSelect();

	gFightHandler.mIsFinalFight = 0;
	gFightHandler.mIsActive = 0;
}


ActorBlueprint getFightHandler()
{
	return makeActorBlueprint(loadFightHandler, NULL, updateFightHandler);
}

static void loadSingleEnemy(string tName, Position tPos) {
	FightEnemy e;

	string path = "enemies/" + tName + ".txt";
	Buffer b = fileToBuffer(path.data());
	BufferPointer p = getBufferPointer(b);

	int animation = readIntegerFromTextStreamBufferPointer(&p);
	e.mAnimationID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), animation), getLevelSprites(), tPos);
	e.mHP = e.mHPMax = readIntegerFromTextStreamBufferPointer(&p);
	e.mSpeed = readIntegerFromTextStreamBufferPointer(&p);
	e.mPosition = tPos;
	e.mName = readLineFromTextStreamBufferPointer(&p);
	e.mName = readLineFromTextStreamBufferPointer(&p);

	int attackAmount = readIntegerFromTextStreamBufferPointer(&p);
	e.mAttacks.clear();
	for (int i = 0; i < attackAmount; i++) {
		FightAttack attack;
		attack.mText = readLineFromTextStreamBufferPointer(&p);
		attack.mText = readLineFromTextStreamBufferPointer(&p);
		attack.mAnimation = readIntegerFromTextStreamBufferPointer(&p);
		attack.mDamage = readIntegerFromTextStreamBufferPointer(&p);
		e.mAttacks.push_back(attack);
	}

	gFightHandler.mEnemies.push_back(e);

	freeBuffer(b);
}

typedef struct {
	string mName;
	Position tPosition;
} LoadEnemy;

static LoadEnemy makeLoadEnemy(string tName, Position tPosition) {
	LoadEnemy e;
	e.mName = tName;
	e.tPosition = tPosition;
	return e;
}

static void findEnemyTeam() {
	vector<vector<LoadEnemy> > possibleEnemies;
	vector<LoadEnemy> list;
	list.push_back(makeLoadEnemy("lamia", makePosition(50, 100, 20)));
	list.push_back(makeLoadEnemy("lamia", makePosition(200, 100, 20)));
	possibleEnemies.push_back(list);

	list.clear();
	list.push_back(makeLoadEnemy("harpy", makePosition(50, 100, 20)));
	list.push_back(makeLoadEnemy("harpy", makePosition(150, 100, 20)));
	list.push_back(makeLoadEnemy("harpy", makePosition(250, 100, 20)));
	possibleEnemies.push_back(list);

	list.clear();
	list.push_back(makeLoadEnemy("mermaid", makePosition(50, 100, 20)));
	list.push_back(makeLoadEnemy("centaur", makePosition(200, 100, 20)));
	possibleEnemies.push_back(list);
	
	list = possibleEnemies[randfromInteger(0, possibleEnemies.size() - 1)];

	gFightHandler.mEnemies.clear();

	for (int i = 0; i < list.size(); i++) {
		LoadEnemy& e = list[i];
		loadSingleEnemy(e.mName, e.tPosition);
	}
}

static void setCharacterUIVisible() {
	for (int i = 0; i < 3; i++) {
		FightCharacter& e = gFightHandler.mCharacters[i];
		if (!e.mIsActive) continue;

		setMugenAnimationVisibility(e.mBGID, 1);

		changeMugenText(e.mNameTextID, e.mName.data());
		updateHealthAndTPText(e);

		setMugenAnimationVisibility(e.mActionAnimationID, 1);

		e.mEnemyTarget = 0;
	}
}

static void setCharacterUIInvisible() {
	for (int i = 0; i < 3; i++) {
		FightCharacter& e = gFightHandler.mCharacters[i];
		if (!e.mIsActive) continue;

		setMugenAnimationVisibility(e.mBGID, 0);

		changeMugenText(e.mNameTextID, "");
		changeMugenText(e.mHPNameTextID, "");
		changeMugenText(e.mHPValueTextID, "");

		changeMugenText(e.mMPNameTextID, "");
		changeMugenText(e.mMPValueTextID, "");

		setMugenAnimationVisibility(e.mActionAnimationID, 0);

	}
}

static void setTotalUIVisible() {
	setMugenAnimationVisibility(gFightHandler.mSelect.mFightBGID, 1);
	setMugenAnimationVisibility(gFightHandler.mSelect.mSelectBGID, 1);
}

static void setTotalUIInvisible() {
	setMugenAnimationVisibility(gFightHandler.mSelect.mFightBGID, 0);
	setMugenAnimationVisibility(gFightHandler.mSelect.mSelectBGID, 0);
}

static void setSelectUIInvisible() {
	changeMugenText(gFightHandler.mSelect.mSelectText1, "");
	changeMugenText(gFightHandler.mSelect.mSelectText2, "");

	setMugenAnimationVisibility(gFightHandler.mSelect.mSelectorID, 0);
	setMugenAnimationVisibility(gFightHandler.mSelect.mCharacterSelectorID, 0);
}

static void setKnownFightActive() {
	setPlayerInactive();

	setCharacterUIVisible();
	setCharactersVisible();
	setTotalUIVisible();
	setSelectUIFightSelect();
	gFightHandler.mSimulation.mIsActive = 0;
	gFightHandler.mIsFightOver = 0;
	gFightHandler.mIsActive = 1;
}

static void setFightActive() {
	findEnemyTeam();
	setKnownFightActive();
}

static void removeEnemies() {
	for (FightEnemy& e : gFightHandler.mEnemies) {
		removeMugenAnimation(e.mAnimationID);
	}

	gFightHandler.mEnemies.clear();
}

static void restoreMinimumCharacterHealth() {
	for (int i = 0; i < 3; i++) {
		gFightHandler.mCharacters[i].mHP = max(1, gFightHandler.mCharacters[i].mHP);
	}
}

static void setFightInactive() {

	setPlayerActive();

	setCharacterUIInvisible();
	setCharactersInvisible();
	removeEnemies();
	setTotalUIInvisible();
	setSelectUIInvisible();
	setStatusTextInvisible();
	restoreMinimumCharacterHealth();
	gFightHandler.mIsActive = 0;

}

void rollFightStart(double tLikeliness)
{
	double perc = randfrom(0, 1);
	if (perc > tLikeliness) return;

	setFightActive();
}

static void resetSingleCharacter(int i, string tName, int tHPMax, int tTPMax, int tSpeed, Position tBackPosition, FightActionType tStartType, FightAttack tDirectAttack, vector<pair<FightAttackDependency, FightAttack> > tMagicAttacks) {
	FightCharacter& e = gFightHandler.mCharacters[i];
	e.mName = tName;

	e.mHP = e.mHPMax = tHPMax;
	e.mTP = e.mTPMax = tTPMax;
	e.mSpeed = tSpeed;
	e.mType = tStartType;
	e.mBackPosition = tBackPosition;
	e.mDirectAttack = tDirectAttack;
	e.mMagicAttacks = tMagicAttacks;
}

static FightAttack makeFightAttack(int tAnimation, int tDamage, string tText) {
	FightAttack e;
	e.mAnimation = tAnimation;
	e.mDamage = tDamage;
	e.mText = tText;
	return e;
}

static FightAttackDependency makeFightAttackDependency(string tName, int tTP) {
	FightAttackDependency e;
	e.tName = tName;
	e.mTP = tTP;
	return e;
}

void resetFightHandler()
{
	vector<pair<FightAttackDependency, FightAttack> > magicAttacks;
	magicAttacks.push_back(make_pair(makeFightAttackDependency("LUX", 30), makeFightAttack(5003, 50, "OSCR shines his light on the enemy!")));
	magicAttacks.push_back(make_pair(makeFightAttackDependency("SLSH", 50), makeFightAttack(5004, 100, "OSCR slashes at $ a little stronger than usual!")));
	resetSingleCharacter(0, "OSCR", 200, 100, 7, makePosition(90, 181, 19), FIGHT_ACTION_FIGHT, makeFightAttack(5000, 50, "OSCR takes a swing at $ with his light-bulb blade!"), magicAttacks);
	magicAttacks.clear();
	magicAttacks.push_back(make_pair(makeFightAttackDependency("AID", 30), makeFightAttack(5005, -50, "PPOM licks $'s wounds!")));
	magicAttacks.push_back(make_pair(makeFightAttackDependency("NYAN", 70), makeFightAttack(5006, 100, "PPOM howls in a cat-and-doggy way!")));
	resetSingleCharacter(1, "PPOM", 70, 200, 10, makePosition(137, 181, 19), FIGHT_ACTION_FIGHT, makeFightAttack(5001, 40, "PPOM scratches $ like a poorly trained pet!"), magicAttacks);

	magicAttacks.clear();
	magicAttacks.push_back(make_pair(makeFightAttackDependency("CURE", 40), makeFightAttack(5005, -70, "LNGA heals the party!")));
	magicAttacks.push_back(make_pair(makeFightAttackDependency("PRAY", 0), makeFightAttack(5007, 0, "LNGA prays for the enemies!")));
	resetSingleCharacter(2, "LNGA", 150, 140, 5, makePosition(199, 181, 19), FIGHT_ACTION_FIGHT, makeFightAttack(5002, 30, "LNGA punches $ in the face! Twice!"), magicAttacks);
}

void setFightPrincessActivity(int tIsActive)
{
	gFightHandler.mCharacters[2].mIsActive = tIsActive;
}

void restoreCharacterHealth()
{
	for (int i = 0; i < 3; i++) {
		gFightHandler.mCharacters[i].mHP = gFightHandler.mCharacters[i].mHPMax;
		gFightHandler.mCharacters[i].mTP = gFightHandler.mCharacters[i].mTPMax;
	}
}

void startFinalFight()
{
	gFightHandler.mIsFinalFight = 1;
	changeMugenAnimation(gFightHandler.mSelect.mFightBGID, getMugenAnimation(getPlayerAnimations(), 3011));
	gFightHandler.mEnemies.clear();
	loadSingleEnemy("witch", makePosition(152, 100, 18));
	setKnownFightActive();
}
