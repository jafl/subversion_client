// This file was automatically generated by jx_menu_editor.
// Do not edit it directly!
// Any changes you make will be silently overwritten.

#ifndef _H_MainDirector_Actions
#define _H_MainDirector_Actions

static const JUtf8Byte* kActionsMenuStr =
"* %i RefreshStatus::SVNMainDirector"
"|* %i CloseTab::SVNMainDirector %l"
"|* %i UpdateWorkingCopy::SVNMainDirector"
"|* %i CleanUpWorkingCopy::SVNMainDirector %l"
"|* %i AddFiles:SVNMainDirector::MainDirector"
"|* %i RemoveFiles:SVNMainDirector::MainDirector"
"|* %i ForceRemoveFiles:SVNMainDirector::MainDirector %l"
"|* %i ResolveSelectedConflicts::SVNMainDirector %l"
"|* %i CommitSelectedFiles::SVNMainDirector"
"|* %i CommitAllFiles::SVNMainDirector %l"
"|* %i RevertSelectedFiles::SVNMainDirector"
"|* %i RevertAllFiles::SVNMainDirector %l"
"|* %i CreateDirectory::SVNMainDirector"
"|* %i DuplicateSelectedItem::SVNMainDirector %l"
"|* %i CreateProperty::SVNMainDirector"
"|* %i RemoveProperties::SVNMainDirector %l"
"|* %i IgnoreSelection::SVNMainDirector"
;

#include "MainDirector-Actions-enum.h"

#ifndef _H_svn_update
#define _H_svn_update
#include "svn_update.xpm"
#endif
#ifndef _H_svn_add
#define _H_svn_add
#include "svn_add.xpm"
#endif
#ifndef _H_svn_remove
#define _H_svn_remove
#include "svn_remove.xpm"
#endif
#ifndef _H_svn_remove_force
#define _H_svn_remove_force
#include "svn_remove_force.xpm"
#endif
#ifndef _H_svn_resolved
#define _H_svn_resolved
#include "svn_resolved.xpm"
#endif
#ifndef _H_svn_commit
#define _H_svn_commit
#include "svn_commit.xpm"
#endif
#ifndef _H_svn_commit_all
#define _H_svn_commit_all
#include "svn_commit_all.xpm"
#endif
#ifndef _H_svn_revert
#define _H_svn_revert
#include "svn_revert.xpm"
#endif
#ifndef _H_svn_revert_all
#define _H_svn_revert_all
#include "svn_revert_all.xpm"
#endif
#ifndef _H_jx_af_image_jx_jx_folder_small
#define _H_jx_af_image_jx_jx_folder_small
#include <jx-af/image/jx/jx_folder_small.xpm>
#endif

static void ConfigureActionsMenu(JXTextMenu* menu, const int offset = 0) {
	if (offset == 0 && JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle) {
		menu->SetShortcuts("#a");
	}
	menu->SetItemImage(kUpdateWorkingCopyCmd + offset, svn_update);
	menu->SetItemImage(kAddSelectedFilesCmd + offset, svn_add);
	menu->SetItemImage(kRemoveSelectedFilesCmd + offset, svn_remove);
	menu->SetItemImage(kForceRemoveSelectedFilesCmd + offset, svn_remove_force);
	menu->SetItemImage(kResolveSelectedConflictsCmd + offset, svn_resolved);
	menu->SetItemImage(kCommitSelectedChangesCmd + offset, svn_commit);
	menu->SetItemImage(kCommitAllChangesCmd + offset, svn_commit_all);
	menu->SetItemImage(kRevertSelectedChangesCmd + offset, svn_revert);
	menu->SetItemImage(kRevertAllChangesCmd + offset, svn_revert_all);
	menu->SetItemImage(kCreateDirectoryCmd + offset, jx_folder_small);
};

#endif
