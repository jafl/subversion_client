/******************************************************************************
 actionDefs.h

	Shared actions for use as menu item ID's and in keybinding tables.
	These preprocessor definitions allow them to be included in static
	menu definitions.

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_actionDefs
#define _H_actionDefs

#include <jx-af/jx/jXActionDefs.h>	// for convenience

// File menu

#define kOpenDirectoryAction       "OpenDirectory::SVNMainDirector"
#define kBrowseRepoAction          "BrowseRepo::SVNMainDirector"
#define kCheckOutRepoAction        "CheckOutRepo::SVNMainDirector"
#define kCheckOutCurrentRepoAction "CheckOutCurrentRepo::SVNMainDirector"
#define kCheckOutSelectionAction   "CheckOutSelection::SVNMainDirector"
#define kOpenFilesAction           "OpenFiles::SVNMainDirector"
#define kShowFilesAction           "ShowFiles::SVNMainDirector"

// Edit menu

#define kCopyFullPathAction "CopyFullPath::SVNMainDirector"

// Actions menu

#define kRefreshStatusAction            "RefreshStatus::SVNMainDirector"
#define kCloseTabAction                 "CloseTab::SVNMainDirector"
#define kUpdateWorkingCopyAction        "UpdateWorkingCopy::SVNMainDirector"
#define kCleanUpWorkingCopyAction       "CleanUpWorkingCopy::SVNMainDirector"
#define kAddFilesAction                 "AddFiles:SVNMainDirector"
#define kRemoveFilesAction              "RemoveFiles:SVNMainDirector"
#define kForceRemoveFilesAction         "ForceRemoveFiles:SVNMainDirector"
#define kResolveSelectedConflictsAction "ResolveSelectedConflicts::SVNMainDirector"
#define kCommitSelectedFilesAction      "CommitSelectedFiles::SVNMainDirector"
#define kCommitAllFilesAction           "CommitAllFiles::SVNMainDirector"
#define kRevertSelectedFilesAction      "RevertSelectedFiles::SVNMainDirector"
#define kRevertAllFilesAction           "RevertAllFiles::SVNMainDirector"
#define kCreateDirectoryAction          "CreateDirectory::SVNMainDirector"
#define kDuplicateSelectedItemAction    "DuplicateSelectedItem::SVNMainDirector"
#define kCreatePropertyAction           "CreateProperty::SVNMainDirector"
#define kRemovePropertiesAction         "RemoveProperties::SVNMainDirector"
#define kIgnoreSelectionAction          "IgnoreSelection::SVNMainDirector"

// Info menu

#define kInfoLogSelectedFilesAction     "InfoLogSelectedFiles::SVNMainDirector"
#define kPropSelectedFilesAction        "PropSelectedFiles::SVNMainDirector"
#define kDiffEditedSelectedFilesAction  "DiffEditedSelectedFiles::SVNMainDirector"
#define kDiffCurrentSelectedFilesAction "DiffCurrentSelectedFiles::SVNMainDirector"
#define kDiffPrevSelectedFilesAction    "DiffPrevSelectedFiles::SVNMainDirector"
#define kCommitDetailsAction            "CommitDetails::SVNMainDirector"
#define kBrowseRevisionAction           "BrowseRevision::SVNMainDirector"
#define kBrowseSelectedRevisionAction   "BrowseSelectedRevision::SVNMainDirector"

#endif
