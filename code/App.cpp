/******************************************************************************
 App.cpp

	BASE CLASS = public JXApplication

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#include "App.h"
#include "AboutDialog.h"
#include "stringData.h"
#include "globals.h"
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jAssert.h>

static const JUtf8Byte* kAppSignature = "nps_svn_client";

/******************************************************************************
 Constructor

 *****************************************************************************/

App::App
	(
	int*		argc,
	char*		argv[],
	bool*		displayAbout,
	JString*	prevVersStr
	)
	:
	JXApplication(argc, argv, kAppSignature, "NPS_SVN_Client", kDefaultStringData)
{
	*displayAbout = CreateGlobals(this);

	if (!*displayAbout)
	{
		*prevVersStr = (GetPrefsManager())->GetPrevVersionStr();
		if (*prevVersStr == GetVersionNumberStr())
		{
			prevVersStr->Clear();
		}
		else
		{
			*displayAbout = true;
		}
	}
	else
	{
		prevVersStr->Clear();
	}
}

/******************************************************************************
 Destructor

 *****************************************************************************/

App::~App()
{
	DeleteGlobals();
}

/******************************************************************************
 Quit (virtual)

 ******************************************************************************/

void
App::Quit()
{
	if (!IsQuitting() && HasPrefsManager())
	{
		GetPrefsManager()->SaveProgramState();
	}

	JXApplication::Quit();
}

/******************************************************************************
 DisplayAbout

	prevVersStr can be nullptr.

 ******************************************************************************/

void
App::DisplayAbout
	(
	const bool		showLicense,
	const JString&	prevVersStr
	)
{
	StartFiber([showLicense, prevVersStr]()
	{
		if (!showLicense || JGetUserNotification()->AcceptLicense())
		{
			auto* dlog = jnew AboutDialog(prevVersStr);
			dlog->DoDialog();

			JCheckForNewerVersion(GetPrefsManager(), kVersionCheckID);
		}
		else
		{
			ForgetPrefsManager();
			JXGetApplication()->Quit();
		}
	},
	"App::DisplayAbout");
}

/******************************************************************************
 ReloadOpenFilesInIDE

 ******************************************************************************/

void
App::ReloadOpenFilesInIDE()
{
	PrefsManager::Integration type;
	JString cmd;
	if (GetPrefsManager()->GetCommand(PrefsManager::kReloadChangedCmd, &type, &cmd))
	{
		JSimpleProcess::Create(cmd, true);
	}
}

/******************************************************************************
 CleanUpBeforeSuddenDeath (virtual protected)

	*** If the server is dead, you cannot call any code that contacts it.

 ******************************************************************************/

void
App::CleanUpBeforeSuddenDeath
	(
	const JXDocumentManager::SafetySaveReason reason
	)
{
	JXApplication::CleanUpBeforeSuddenDeath(reason);

	if (reason != JXDocumentManager::kAssertFired)
	{
		GetPrefsManager()->SaveProgramState();
	}

	::CleanUpBeforeSuddenDeath(reason);		// must be last call
}

/******************************************************************************
 GetAppSignature (static)

 ******************************************************************************/

const JUtf8Byte*
App::GetAppSignature()
{
	return kAppSignature;
}

/******************************************************************************
 InitStrings (static)

	If we are going to print something to the command line and then quit,
	we haven't initialized JX, but we still need the string data.

 ******************************************************************************/

void
App::InitStrings()
{
	JGetStringManager()->Register(kAppSignature, kDefaultStringData);
}
