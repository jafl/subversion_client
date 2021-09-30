/******************************************************************************
 menus.h

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#ifndef _H_menus
#define _H_menus

#include "actionDefs.h"

enum ActionMenu
{
	kRefreshCmd = 1,
	kCloseTabCmd,
	kUpdateWorkingCopyCmd,
	kCleanUpWorkingCopyCmd,
	kAddSelectedFilesCmd,
	kRemoveSelectedFilesCmd,
	kForceRemoveSelectedFilesCmd,
	kResolveSelectedConflictsCmd,
	kCommitSelectedChangesCmd,
	kCommitAllChangesCmd,
	kRevertSelectedChangesCmd,
	kRevertAllChangesCmd,
	kCreateDirectoryCmd,
	kDuplicateSelectedItemCmd,
	kCreatePropertyCmd,
	kRemoveSelectedPropertiesCmd,
	kIgnoreSelectionCmd
};

enum InfoMenu
{
	kInfoLogSelectedFilesCmd = 1,
	kPropSelectedFilesCmd,
	kDiffEditedSelectedFilesCmd,
	kDiffCurrentSelectedFilesCmd,
	kDiffPrevSelectedFilesCmd,
	kCommitDetailsCmd,
	kBrowseRepoRevisionCmd,
	kBrowseSelectedRepoRevisionCmd
};

#endif
