/******************************************************************************
 globals.cpp

	Access to global objects and factories.

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "globals.h"
#include "App.h"
#include "PrefsManager.h"
#include "WDManager.h"
#include "MDIServer.h"
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jcore/jAssert.h>

static App*				theApplication  = nullptr;		// owns itself
static PrefsManager*	thePrefsManager = nullptr;
static WDManager*		theWDManager    = nullptr;		// owned by JX
static MDIServer*		theMDIServer    = nullptr;

/******************************************************************************
 CreateGlobals

	Returns true if this is the first time the program is run.

 ******************************************************************************/

bool
CreateGlobals
	(
	App* app
	)
{
	theApplication = app;

	JXDisplay* display = app->GetCurrentDisplay();

	bool isNew;
	thePrefsManager	= jnew PrefsManager(&isNew);

	JXInitHelp();

	theWDManager = jnew WDManager(display, true);

	theMDIServer = jnew MDIServer;

	return isNew;
}

/******************************************************************************
 DeleteGlobals

 ******************************************************************************/

void
DeleteGlobals()
{
	theApplication = nullptr;
	theMDIServer   = nullptr;
	theWDManager   = nullptr;

	JXFSBindingManager::Destroy();

	// this must be last so everybody else can use it to save their setup

	jdelete thePrefsManager;
	thePrefsManager = nullptr;
}

/******************************************************************************
 CleanUpBeforeSuddenDeath

	This must be the last one called by App so we can save
	the preferences to disk.

	*** If the server is dead, you cannot call any code that contacts it.

 ******************************************************************************/

void
CleanUpBeforeSuddenDeath
	(
	const JXDocumentManager::SafetySaveReason reason
	)
{
	if (reason != JXDocumentManager::kAssertFired)
	{
//		theMDIServer->JPrefObject::WritePrefs();
	}

	// must be last to save everything

	thePrefsManager->CleanUpBeforeSuddenDeath(reason);
}

/******************************************************************************
 GetApplication

 ******************************************************************************/

App*
GetApplication()
{
	assert( theApplication != nullptr );
	return theApplication;
}

/******************************************************************************
 HasPrefsManager

 ******************************************************************************/

bool
HasPrefsManager()
{
	return thePrefsManager != nullptr;
}

/******************************************************************************
 GetPrefsManager

 ******************************************************************************/

PrefsManager*
GetPrefsManager()
{
	assert( thePrefsManager != nullptr );
	return thePrefsManager;
}

/******************************************************************************
 ForgetPrefsManager

	Called when license is not accepted, to avoid writing prefs file.

 ******************************************************************************/

void
ForgetPrefsManager()
{
	thePrefsManager = nullptr;
}

/******************************************************************************
 GetWDManager

 ******************************************************************************/

WDManager*
GetWDManager()
{
	assert( theWDManager != nullptr );
	return theWDManager;
}

/******************************************************************************
 GetMDIServer

 ******************************************************************************/

MDIServer*
GetMDIServer()
{
	assert( theMDIServer != nullptr );
	return theMDIServer;
}

/******************************************************************************
 GetVersionNumberStr

 ******************************************************************************/

const JString&
GetVersionNumberStr()
{
	return JGetString("VERSION");
}

/******************************************************************************
 GetVersionStr

 ******************************************************************************/

JString
GetVersionStr()
{
	const JUtf8Byte* map[] =
	{
		"version",   JGetString("VERSION").GetBytes(),
		"copyright", JGetString("COPYRIGHT").GetBytes()
	};
	return JGetString("Description::globals", map, sizeof(map));
}

/******************************************************************************
 GetWMClassInstance

 ******************************************************************************/

const JUtf8Byte*
GetWMClassInstance()
{
	return "NPS_SVN_Client";
}

const JUtf8Byte*
GetMainWindowClass()
{
	return "NPS_SVN_Client_Main_Window";
}
