/******************************************************************************
 main.cpp

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "App.h"
#include "MDIServer.h"
#include "MainDirector.h"
#include "globals.h"
#include <jx-af/jcore/jCommandLine.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jTime.h>
#include <jx-af/jcore/jAssert.h>

// Prototypes

void ParseTextOptions(const int argc, char* argv[]);
void PrintVersion();

/******************************************************************************
 main

 ******************************************************************************/

int
main
	(
	int		argc,
	char*	argv[]
	)
{
	ParseTextOptions(argc, argv);

	if (!MDIServer::WillBeMDIServer(App::GetAppSignature(), argc, argv))
	{
		return 0;
	}

	bool displayAbout;
	JString prevVersStr;
	auto* app = jnew App(&argc, argv, &displayAbout, &prevVersStr);
	assert( app != nullptr );

	if (displayAbout &&
		!JGetUserNotification()->AcceptLicense())
	{
		return 0;
	}

	JCheckForNewerVersion(GetPrefsManager(), kVersionCheckID);

	(GetMDIServer())->HandleCmdLineOptions(argc, argv);

	if (displayAbout)
	{
		app->DisplayAbout(prevVersStr);
	}

	app->Run();
	return 0;
}

/******************************************************************************
 ParseTextOptions

	Handle the command line options that don't require opening an X display.

	Options that don't call exit() should use JXApplication::RemoveCmdLineOption()
	so ParseXOptions won't gag.

 ******************************************************************************/

void
ParseTextOptions
	(
	const int	argc,
	char*		argv[]
	)
{
	long index = 1;
	while (index < argc)
	{
		if (JIsVersionRequest(argv[index]))
		{
			App::InitStrings();
			PrintVersion();
			exit(0);
		}
		else if (JIsHelpRequest(argv[index]))
		{
			App::InitStrings();
			MDIServer::PrintCommandLineHelp();
			exit(0);
		}
		index++;
	}
}

/******************************************************************************
 PrintVersion

 ******************************************************************************/

void
PrintVersion()
{
	std::cout << std::endl;
	std::cout << GetVersionStr() << std::endl;
	std::cout << std::endl;
}
