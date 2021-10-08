/******************************************************************************
 WDManager.h

	Copyright (C) 1997-2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_WDManager
#define _H_WDManager

#include <jx-af/jx/JXWDManager.h>

class MainDirector;

class WDManager : public JXWDManager
{
public:

	WDManager(JXDisplay* display, const bool wantShortcuts);

	~WDManager() override;

	bool			NewBrowser(MainDirector** dir);
	bool			GetBrowser(const JString& path, MainDirector** dir);
	bool			GetBrowserForExactURL(const JString& url, MainDirector** dir);
	MainDirector*	OpenDirectory(const JString& path, bool* wasOpen = nullptr);

	// called by CBPrefsManager

	bool	RestoreState(std::istream& input);
	bool	SaveState(std::ostream& output) const;
};

#endif
