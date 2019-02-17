#include <prism/framerateselectscreen.h>
#include <prism/pvr.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/logoscreen.h>
#include <prism/mugentexthandler.h>

#include "overworldscreen.h"
#include "storyhandler.h"
#include "titlescreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#endif

// #define DEVELOP

void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	if (isOnDreamcast()) {
		abortSystem();
	}
	else {
		returnToMenu();
	}
#else
	returnToMenu();
#endif
}

void setMainFileSystem() {
#ifdef DEVELOP
	setFileSystem("/pc");
#else
	setFileSystem("/cd");
#endif
}

int isInDevelopMode() {
#ifdef DEVELOP
	return 1;
#else
	return 0;
#endif

}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	setGameName("Schildburg");
	setScreenSize(320, 240);
	
	setMainFileSystem();
	initPrismWrapperWithConfigFile("data/config.cfg");
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	addMugenFont(-1, "font/jg.fnt");
	addMugenFont(1, "font/jg.fnt");
	addMugenFont(2, "font/name1.fnt");
	addMugenFont(3, "font/f6x9.fnt");
	addMugenFont(4, "font/f4x6.fnt");

	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}

	// setDisplayedScreenSize(320, 340);
	// disableWrapperErrorRecovery();
	setWrapperTitleScreen(getTitleScreen());
	setScreenAfterWrapperLogoScreen(getLogoScreenFromWrapper());
	startScreenHandling(getTitleScreen());

	exitGame();
	
	return 0;
}


