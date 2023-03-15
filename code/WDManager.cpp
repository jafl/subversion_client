/******************************************************************************
 WDManager.cpp

	BASE CLASS = JXWDManager

	Copyright (C) 1997-2008 by John Lindal.

 ******************************************************************************/

#include "WDManager.h"
#include "MainDirector.h"
#include <jx-af/jx/JXChoosePathDialog.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jAssert.h>

// state information

const JFileVersion kCurrentStateVersion = 1;

	// version  1 restores open nodes in repo view

/******************************************************************************
 Constructor

 ******************************************************************************/

WDManager::WDManager
	(
	JXDisplay*	display,
	const bool	wantShortcuts
	)
	:
	JXWDManager(display, wantShortcuts)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

WDManager::~WDManager()
{
}

/******************************************************************************
 NewBrowser

 *****************************************************************************/

void
WDManager::NewBrowser()
{
	auto* dlog = JXChoosePathDialog::Create(JXChoosePathDialog::kRequireWritable, JString::empty, JString::empty, JGetString("NewBrowserInstr::WDManager"));
	if (dlog->DoDialog())
	{
		OpenDirectory(dlog->GetPath());
	}
}

/******************************************************************************
 GetBrowser

 *****************************************************************************/

bool
WDManager::GetBrowser
	(
	const JString&	path,
	MainDirector**	dir
	)
{
	*dir = nullptr;

	JString p1;
	bool isURL = JIsURL(path);
	if (isURL)
	{
		p1 = path;
		JAppendDirSeparator(&p1);
	}
	else if (!JGetTrueName(path, &p1))
	{
		return false;
	}

	JPtrArray<JXWindowDirector> windowList(JPtrArrayT::kForgetAll);
	GetDirectors(&windowList);

	const JSize windowCount = windowList.GetElementCount();
	JString p2, p3;
	for (JIndex i=1; i<=windowCount; i++)
	{
		auto* d = dynamic_cast<MainDirector*>(windowList.GetElement(i));
		if (d == nullptr)
		{
			continue;
		}

		if (isURL && d->GetRepoPath(&p2))
		{
			JAppendDirSeparator(&p2);
			if (p1.StartsWith(p2))
			{
				*dir = d;
				break;
			}
		}
		else if (!isURL && d->GetPath(&p3) &&
				 (JSameDirEntry(p1, p3) ||
				  (JGetTrueName(p3, &p2) && p1.StartsWith(p2))))
		{
			*dir = d;
			break;
		}
	}

	return *dir != nullptr;
}

/******************************************************************************
 GetBrowserForExactURL

 *****************************************************************************/

bool
WDManager::GetBrowserForExactURL
	(
	const JString&		url,
	MainDirector**	dir
	)
{
	*dir = nullptr;

	JString p1 = url;
	JStripTrailingDirSeparator(&p1);

	JPtrArray<JXWindowDirector> windowList(JPtrArrayT::kForgetAll);
	GetDirectors(&windowList);

	const JSize windowCount = windowList.GetElementCount();
	JString p2;
	for (JIndex i=1; i<=windowCount; i++)
	{
		auto* d = dynamic_cast<MainDirector*>(windowList.GetElement(i));
		if (d != nullptr && d->GetRepoPath(&p2) && p1 == p2)
		{
			*dir = d;
			break;
		}
	}

	return *dir != nullptr;
}

/******************************************************************************
 OpenDirectory

 *****************************************************************************/

MainDirector*
WDManager::OpenDirectory
	(
	const JString&	path,
	bool*		wasOpen		// can be nullptr
	)
{
	MainDirector* dir;
	const bool open = GetBrowser(path, &dir);
	if (wasOpen != nullptr)
	{
		*wasOpen = open;
	}

	if (!open)
	{
		dir = jnew MainDirector(JXGetApplication(), path);
		assert( dir != nullptr );
	}

	dir->Activate();
	return dir;
}

/******************************************************************************
 RestoreState

	Reopens files that were open when we quit the last time.  Returns false
	if nothing is opened.

 ******************************************************************************/

bool
WDManager::RestoreState
	(
	std::istream& input
	)
{
	JFileVersion vers;
	input >> vers;
	if (vers > kCurrentStateVersion)
	{
		return false;
	}

	JSize windowCount;
	input >> windowCount;

	JString fileName;
	for (JIndex i=1; i<=windowCount; i++)
	{
		auto* dir = jnew MainDirector(JXGetApplication(), input, vers);
		assert( dir != nullptr );
		dir->Activate();
	}

	return windowCount > 0;
}

/******************************************************************************
 SaveState

	Saves files that are currently open so they can be reopened next time.
	Returns false if there is nothing to save.

	Always calls WriteForProject() because project documents might be
	write protected, e.g., CVS.

 ******************************************************************************/

bool
WDManager::SaveState
	(
	std::ostream& output
	)
	const
{
	JPtrArray<JXWindowDirector> windowList(JPtrArrayT::kForgetAll);
	GetDirectors(&windowList);
	if (windowList.IsEmpty())
	{
		return false;
	}

	output << kCurrentStateVersion;

	const JSize windowCount = windowList.GetElementCount();
	output << ' ' << windowCount;

	for (JIndex i=1; i<=windowCount; i++)
	{
		auto* dir = dynamic_cast<MainDirector*>(windowList.GetElement(i));

		output << ' ';
		dir->StreamOut(output);
	}

	return true;
}
