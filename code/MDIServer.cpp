/******************************************************************************
 MDIServer.cpp

	BASE CLASS = public JXMDIServer

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#include "MDIServer.h"
#include "MainDirector.h"
#include "RepoView.h"
#include "RepoTree.h"
#include "globals.h"
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/JStringMatch.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 *****************************************************************************/

MDIServer::MDIServer()
	:
	JXMDIServer()
{
}

/******************************************************************************
 Destructor

 *****************************************************************************/

MDIServer::~MDIServer()
{
}

/******************************************************************************
 HandleMDIRequest

 *****************************************************************************/

enum Action
{
	kStatus,
	kRefreshRepo,
	kRefreshStatus,
	kUpdate,
	kInfoLog,
	kPropList
};

void
MDIServer::HandleMDIRequest
	(
	const JString&				dir,
	const JPtrArray<JString>&	argList
	)
{
	const JSize argCount = argList.GetElementCount();

	JString s, fullPath;
	Action action = kStatus;

	bool restore       = IsFirstTime();
	bool failedRefresh = false;
	for (JIndex i=2; i<=argCount; i++)
	{
		const JString& arg = *argList.GetElement(i);
		if (arg == "--status")
		{
			action = kStatus;
		}
		else if (arg == "--refresh-repo")
		{
			action = kRefreshRepo;
		}
		else if (arg == "--refresh-status")
		{
			action = kRefreshStatus;
		}
		else if (arg == "--update")
		{
			action = kUpdate;
		}
		else if (arg == "--info-log")
		{
			action = kInfoLog;
		}
		else if (arg == "--proplist")
		{
			action = kPropList;
		}
		else
		{
			bool isURL = JIsURL(arg);
			JString rev;
			if (isURL)
			{
				fullPath = arg;

				JStringIterator iter(&fullPath, JStringIterator::kStartAtEnd);
				iter.BeginMatch();
				if (iter.Prev("@"))
				{
					rev = iter.FinishMatch().GetString();
					if (rev.Contains("/"))
					{
						rev.Clear();	// found username instead
					}
					else
					{
						iter.RemoveAllNext();
					}
				}
			}
			else if (!JConvertToAbsolutePath(arg, dir, &s) ||
					 !JGetTrueName(s, &fullPath))
			{
				const JUtf8Byte* map[] =
				{
					"path", arg.GetBytes()
				};
				JGetUserNotification()->ReportError(JGetString("PathNotFound::MDIServer", map, sizeof(map)));
				continue;
			}

			if (action == kRefreshRepo || action == kRefreshStatus)
			{
				MainDirector* dir1;
				const bool open = (GetWDManager())->GetBrowser(fullPath, &dir1);
				if (open && !dir1->OKToStartActionProcess())
				{
					JGetUserNotification()->ReportError(JGetString("WindowBusy::MDIServer"));
				}
				else if (open && action == kRefreshRepo)
				{
					dir1->RefreshRepo();
				}
				else if (open && action == kRefreshStatus)
				{
					dir1->RefreshStatus();
				}
				else
				{
					failedRefresh = true;
				}
			}
			else
			{
				JString path = fullPath, name;
				if (isURL)
				{
					JXGetApplication()->DisplayBusyCursor();

					JString type, error;
					if (!JGetSVNEntryType(path, &type, &error))
					{
						JGetUserNotification()->ReportError(error);
						continue;
					}
					else if (type != "dir")
					{
						JStripTrailingDirSeparator(&fullPath);
						JSplitPathAndName(fullPath, &path, &name);
					}
				}
				else if (JFileExists(path))
				{
					JStripTrailingDirSeparator(&fullPath);
					JSplitPathAndName(fullPath, &path, &name);
				}

				bool wasOpen;
				MainDirector* dir1 = GetWDManager()->OpenDirectory(path, &wasOpen);
				if (action != kStatus && action != kInfoLog &&
					!dir1->OKToStartActionProcess())
				{
					JGetUserNotification()->ReportError(JGetString("WindowBusy::MDIServer"));
				}
				else if (action == kUpdate)
				{
					dir1->UpdateWorkingCopy();
				}
				else if (action == kInfoLog)
				{
					dir1->ShowInfoLog(fullPath);
				}
				else if (action == kPropList)
				{
					dir1->ShowProperties(fullPath);
				}
				else if (isURL && !rev.IsEmpty())
				{
					dir1->BrowseRepo(rev);
				}
				else if (wasOpen)
				{
					dir1->RefreshRepo();
					dir1->RefreshStatus();

					RepoView* widget;
					if (isURL && dir1->GetRepoWidget(&widget))
					{
						widget->GetRepoTree()->SavePathToOpen(path);
					}
				}

				restore = false;
			}
		}
	}

	// if argCount == 1, restore guaranteed to be true the first time

	if (restore && failedRefresh)
	{
		exit(0);	// don't lose state
	}
	else if (restore && !GetPrefsManager()->RestoreProgramState())
	{
		JScheduleTask([]()
		{
			GetWDManager()->NewBrowser();
		});
	}
}

/******************************************************************************
 PrintCommandLineHelp (static)

 ******************************************************************************/

void
MDIServer::PrintCommandLineHelp()
{
	const JUtf8Byte* map[] =
	{
		"vers",      GetVersionNumberStr().GetBytes(),
		"copyright", JGetString("COPYRIGHT").GetBytes()
	};
	const JString s = JGetString("CommandLineHelp::MDIServer", map, sizeof(map));
	std::cout << std::endl << s << std::endl << std::endl;
}
