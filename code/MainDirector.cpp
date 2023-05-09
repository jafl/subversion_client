/******************************************************************************
 MainDirector.cpp

	BASE CLASS = public JXWindowDirector

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#include "MainDirector.h"
#include "TabGroup.h"
#include "RepoView.h"
#include "RepoTree.h"
#include "GetRepoDialog.h"
#include "StatusList.h"
#include "UpdateList.h"
#include "InfoLog.h"
#include "PropertiesList.h"
#include "CommandLog.h"
#include "RefreshStatusTask.h"
#include "MDIServer.h"
#include "menus.h"
#include "globals.h"
#include <jx-af/jx/JXHelpManager.h>
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jx/JXWebBrowser.h>
#include <jx-af/jx/JXMacWinPrefsDialog.h>
#include <jx-af/jx/JXGetStringDialog.h>
#include <jx-af/jx/JXSaveFileDialog.h>
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXWDMenu.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXScrollbarSet.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

// File menu

static const JUtf8Byte* kFileMenuStr =
	"    Open directory...                   %k Meta-O                   %i" kOpenDirectoryAction
	"  | Browse repository...                %k Meta-Shift-O             %i" kBrowseRepoAction
	"%l| Check out repository...                                         %i" kCheckOutRepoAction
	"  | Check out current repository...                                 %i" kCheckOutCurrentRepoAction
	"  | Check out selected directory...                                 %i" kCheckOutSelectionAction
	"%l| Open selected files                 %k Left-dbl-click or Return %i" kOpenFilesAction
	"  | Show selected files in file manager                             %i" kShowFilesAction
	"%l| Close                               %k Meta-W                   %i" kJXCloseWindowAction
	"%l| Quit                                %k Meta-Q                   %i" kJXQuitAction;

enum
{
	kOpenDirectoryCmd = 1,
	kBrowseRepoCmd,
	kCheckOutRepoCmd,
	kCheckOutCurrentRepoCmd,
	kCheckOutSelectionCmd,
	kOpenFilesCmd,
	kShowFilesCmd,
	kCloseCmd,
	kQuitCmd
};

// Edit menu additions

static const JUtf8Byte* kEditMenuAddStr =
	"Copy full path %k Meta-Shift-C %i" kCopyFullPathAction;

// Actions menu

static const JUtf8Byte* kActionsMenuStr =
	"    Refresh display            %k F5     %i" kRefreshStatusAction
	"  | Close tab                  %k Ctrl-W %i" kCloseTabAction
	"%l| Update working copy        %k Meta-@ %i" kUpdateWorkingCopyAction
	"  | Clean up working copy                %i" kCleanUpWorkingCopyAction
	"%l| Add selected files         %k Meta-! %i" kAddFilesAction
	"  | Remove selected files                %i" kRemoveFilesAction
	"  | Remove selected files (force)        %i" kForceRemoveFilesAction
	"%l| Resolve selected conflicts %k Meta-` %i" kResolveSelectedConflictsAction
	"%l| Commit selected changes    %k Meta-~ %i" kCommitSelectedFilesAction
	"  | Commit all changes                   %i" kCommitAllFilesAction
	"%l| Revert selected changes              %i" kRevertSelectedFilesAction
	"  | Revert all changes                   %i" kRevertAllFilesAction
	"%l| New directory...                     %i" kCreateDirectoryAction
	"  | Duplicate selected file/directory... %i" kDuplicateSelectedItemAction
	"%l| New property...                      %i" kCreatePropertyAction
	"  | Remove selected properties...        %i" kRemovePropertiesAction
	"%l| Ignore selected item...              %i" kIgnoreSelectionAction;

	/*
	 * Remember to keeup menus.h in sync
	 */

// Info menu

static const JUtf8Byte* kInfoMenuStr =
	"    Info & Log %k Meta-I     %i" kInfoLogSelectedFilesAction
	"  | Properties               %i" kPropSelectedFilesAction
	"%l| Compare with edited      %i" kDiffEditedSelectedFilesAction
	"  | Compare with current     %i" kDiffCurrentSelectedFilesAction
	"  | Compare with previous    %i" kDiffPrevSelectedFilesAction
	"%l| Commit details           %i" kCommitDetailsAction
	"%l| Browse revision...       %i" kBrowseRevisionAction
	"  | Browse selected revision %i" kBrowseSelectedRevisionAction;

	/*
	 * Remember to keeup menus.h in sync
	 */

// Misc menus

static const JUtf8Byte* kPrefsMenuStr =
	"    Integration..."
	"  | Edit tool bar..."
	"  | File bindings..."
	"  | File manager & web browser..."
	"  | Mac/Win/X emulation..."
	"%l| Save window setup as default";

enum
{
	kIntegrationPrefsCmd = 1,
	kEditToolBarCmd,
	kEditBindingsCmd,
	kWebBrowserCmd,
	kEditMacWinPrefsCmd,
	kSaveWindSizeCmd
};

static const JUtf8Byte* kHelpMenuStr =
	"    About"
	"%l| Table of Contents       %i" kJXHelpTOCAction
	"  | Overview"
	"  | This window       %k F1 %i" kJXHelpSpecificAction
	"%l| Changes"
	"  | Credits";

enum
{
	kAboutCmd = 1,
	kTOCCmd,
	kOverviewCmd,
	kThisWindowCmd,
	kChangesCmd,
	kCreditsCmd
};

/******************************************************************************
 Constructor

 *****************************************************************************/

MainDirector::MainDirector
	(
	JXDirector*		supervisor,
	const JString&	path
	)
	:
	JXWindowDirector(supervisor),
	itsPath(path)
{
	MainDirectorX();

	JPoint desktopLoc;
	JCoordinate w,h;
	if ((GetPrefsManager())->GetWindowSize(kMainDirectorWindSizeID,
											  &desktopLoc, &w, &h))
	{
		JXWindow* window = GetWindow();
		window->Place(desktopLoc.x, desktopLoc.y);
		window->SetSize(w,h);
	}

	if (itsRepoWidget != nullptr)
	{
		itsRepoWidget->RefreshContent();
	}
}

MainDirector::MainDirector
	(
	JXDirector*		supervisor,
	std::istream&	input,
	JFileVersion	vers
	)
	:
	JXWindowDirector(supervisor)
{
	input >> itsPath;

	MainDirectorX();

	GetWindow()->ReadGeometry(input);

	if (vers >= 1)
	{
		bool hadRepoWidget;
		input >> JBoolFromString(hadRepoWidget);

		if (itsRepoWidget != nullptr)
		{
			itsRepoWidget->ReadSetup(hadRepoWidget, input, vers);
		}
		else if (hadRepoWidget)
		{
			RepoView::SkipSetup(input, vers);
		}
	}
}

// private

void
MainDirector::MainDirectorX()
{
	itsRepoWidget        = nullptr;
	itsStatusWidget      = nullptr;
	itsActionProcess     = nullptr;
	itsCheckOutProcess   = nullptr;
	itsRefreshStatusTask = nullptr;

	itsTabList = jnew JPtrArray<TabBase>(JPtrArrayT::kForgetAll);
	assert( itsTabList != nullptr );

	BuildWindow();

	GetDisplay()->GetWDManager()->DirectorCreated(this);
}

/******************************************************************************
 Destructor

 *****************************************************************************/

MainDirector::~MainDirector()
{
	jdelete itsTabList;
}

/******************************************************************************
 StreamOut

 *****************************************************************************/

void
MainDirector::StreamOut
	(
	std::ostream& output
	)
	const
{
	// before BuildWindow()

	if (HasPath())
	{
		output << itsPath;
	}
	else
	{
		JString repoPath;
		const bool hasRepo = GetRepoPath(&repoPath);
		assert( hasRepo );
		output << repoPath;
	}

	// after BuildWindow()

	output << ' ';
	GetWindow()->WriteGeometry(output);

	output << ' ' << JBoolToString(itsRepoWidget != nullptr);

	if (itsRepoWidget != nullptr)
	{
		output << ' ';
		itsRepoWidget->WriteSetup(output);
	}
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

#include "svn_main_window_icon.xpm"
#include "svn_update.xpm"
#include "svn_add.xpm"
#include "svn_remove.xpm"
#include "svn_remove_force.xpm"
#include "svn_resolved.xpm"
#include "svn_commit.xpm"
#include "svn_commit_all.xpm"
#include "svn_revert.xpm"
#include "svn_revert_all.xpm"
#include <jx-af/image/jx/jx_folder_small.xpm>
#include "svn_info_log.xpm"
#include <jx-af/image/jx/jx_help_specific.xpm>
#include <jx-af/image/jx/jx_help_toc.xpm>

void
MainDirector::BuildWindow()
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 500,300, JString::empty);
	assert( window != nullptr );

	auto* menuBar =
		jnew JXMenuBar(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 500,30);
	assert( menuBar != nullptr );

	itsToolBar =
		jnew JXToolBar(GetPrefsManager(), kMainToolBarID, menuBar, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,270);
	assert( itsToolBar != nullptr );

// end JXLayout

	const bool isURL = JIsURL(itsPath);

	JString displayPath;
	if (isURL)
	{
		displayPath = itsPath;
	}
	else
	{
		displayPath = JConvertToHomeDirShortcut(itsPath);
	}
	UpdateWindowTitle(displayPath);
	window->SetMinSize(200, 200);
	window->SetWMClass(GetWMClassInstance(), GetMainWindowClass());

	auto* image = jnew JXImage(GetDisplay(), svn_main_window_icon);
	assert( image != nullptr );
	window->SetIcon(image);
	ListenTo(window);

	// menus

	itsFileMenu = menuBar->AppendTextMenu(JGetString("FileMenuTitle::JXGlobal"));
	itsFileMenu->SetMenuItems(kFileMenuStr, "MainDirector");
	itsFileMenu->AttachHandlers(this,
		&MainDirector::UpdateFileMenu,
		&MainDirector::HandleFileMenu);

	itsEditMenu = JXTEBase::StaticAppendEditMenu(menuBar, false, false, false, false, false, false, false, false);

	const JSize count = itsEditMenu->GetItemCount();
	for (JIndex i=1; i<count; i++)
	{
		const JString* id;
		if (itsEditMenu->GetItemID(i, &id) && *id == kJXCopyAction)
		{
			itsEditMenu->InsertMenuItems(i+1, kEditMenuAddStr, "MainDirector");
			break;
		}
	}

	itsActionsMenu = menuBar->AppendTextMenu(JGetString("ActionsMenuTitle::MainDirector"));
	itsActionsMenu->SetMenuItems(kActionsMenuStr, "MainDirector");
	itsActionsMenu->AttachHandlers(this,
		&MainDirector::UpdateActionsMenu,
		&MainDirector::HandleActionsMenu);

	itsActionsMenu->SetItemImage(kUpdateWorkingCopyCmd,        svn_update);
	itsActionsMenu->SetItemImage(kAddSelectedFilesCmd,         svn_add);
	itsActionsMenu->SetItemImage(kRemoveSelectedFilesCmd,      svn_remove);
	itsActionsMenu->SetItemImage(kForceRemoveSelectedFilesCmd, svn_remove_force);
	itsActionsMenu->SetItemImage(kResolveSelectedConflictsCmd, svn_resolved);
	itsActionsMenu->SetItemImage(kCommitSelectedChangesCmd,    svn_commit);
	itsActionsMenu->SetItemImage(kCommitAllChangesCmd,         svn_commit_all);
	itsActionsMenu->SetItemImage(kRevertSelectedChangesCmd,    svn_revert);
	itsActionsMenu->SetItemImage(kRevertAllChangesCmd,         svn_revert_all);
	itsActionsMenu->SetItemImage(kCreateDirectoryCmd,          jx_folder_small);

	itsInfoMenu = menuBar->AppendTextMenu(JGetString("InfoMenuTitle::MainDirector"));
	itsInfoMenu->SetMenuItems(kInfoMenuStr, "MainDirector");
	itsInfoMenu->AttachHandlers(this,
		&MainDirector::UpdateInfoMenu,
		&MainDirector::HandleInfoMenu);

	itsInfoMenu->SetItemImage(kInfoLogSelectedFilesCmd, svn_info_log);

	auto* wdMenu =
		jnew JXWDMenu(JGetString("WindowsMenuTitle::JXGlobal"), menuBar,
					 JXWidget::kFixedLeft, JXWidget::kVElastic, 0,0, 10,10);
	assert( wdMenu != nullptr );
	menuBar->AppendMenu(wdMenu);

	itsPrefsMenu = menuBar->AppendTextMenu(JGetString("PrefsMenuTitle::JXGlobal"));
	itsPrefsMenu->SetMenuItems(kPrefsMenuStr, "MainDirector");
	itsPrefsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsPrefsMenu->AttachHandler(this, &MainDirector::HandlePrefsMenu);

	itsHelpMenu = menuBar->AppendTextMenu(JGetString("HelpMenuTitle::JXGlobal"));
	itsHelpMenu->SetMenuItems(kHelpMenuStr, "MainDirector");
	itsHelpMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsHelpMenu->AttachHandler(this, &MainDirector::HandleHelpMenu);

	itsHelpMenu->SetItemImage(kTOCCmd,        jx_help_toc);
	itsHelpMenu->SetItemImage(kThisWindowCmd, jx_help_specific);

	// tab group

	itsTabGroup =
		jnew TabGroup(itsToolBar->GetWidgetEnclosure(),
						JXWidget::kHElastic, JXWidget::kVElastic,
						0,0, 100,100);
	assert( itsTabGroup != nullptr );
	itsTabGroup->FitToEnclosure();
	ListenTo(itsTabGroup->GetCardEnclosure());

	// repository view

	JString repoPath = itsPath;
	if (isURL || JGetVCSRepositoryPath(itsPath, &repoPath))
	{
		JXContainer* card            = itsTabGroup->AppendTab(JGetString("RepoTab::MainDirector"));
		JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

		itsRepoWidget =
			jnew RepoView(this, repoPath, JString::empty, itsEditMenu,
							scrollbarSet, scrollbarSet->GetScrollEnclosure(),
							JXWidget::kHElastic, JXWidget::kVElastic,
							0,0, 100,100);
		assert( itsRepoWidget != nullptr );

		itsTabList->Append(itsRepoWidget);
	}

	// svn status

	if (!isURL)
	{
		JXContainer* card            = itsTabGroup->AppendTab(JGetString("StatusTab::MainDirector"));
		JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

		itsStatusWidget =
			jnew StatusList(this, itsEditMenu,
							  scrollbarSet, scrollbarSet->GetScrollEnclosure(),
							  JXWidget::kHElastic, JXWidget::kVElastic,
							  0,0, 100,100);
		assert( itsStatusWidget != nullptr );

		itsTabList->Append(itsStatusWidget);
		itsStatusWidget->RefreshContent();

		itsTabGroup->ShowTab(card);
	}

	// if we opened a URL, disable all local functionality

	if (isURL)
	{
		itsPath.Clear();
	}

	// must be done after creating widgets

	itsToolBar->LoadPrefs();
	if (itsToolBar->IsEmpty())
	{
		itsToolBar->AppendButton(itsActionsMenu, kUpdateWorkingCopyCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsActionsMenu, kAddSelectedFilesCmd);
		itsToolBar->AppendButton(itsActionsMenu, kRemoveSelectedFilesCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsActionsMenu, kResolveSelectedConflictsCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsActionsMenu, kCommitSelectedChangesCmd);
		itsToolBar->AppendButton(itsActionsMenu, kCommitAllChangesCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsActionsMenu, kRevertSelectedChangesCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsInfoMenu, kInfoLogSelectedFilesCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsHelpMenu, kTOCCmd);
		itsToolBar->AppendButton(itsHelpMenu, kThisWindowCmd);
	}
}

/******************************************************************************
 BuildScrollbarSet (private)

 ******************************************************************************/

JXScrollbarSet*
MainDirector::BuildScrollbarSet
	(
	JXContainer* widget
	)
{
	auto* scrollbarSet =
		jnew JXScrollbarSet(widget, JXWidget::kHElastic,JXWidget::kVElastic,
						   0,0, 100,100);
	assert( scrollbarSet != nullptr );
	scrollbarSet->FitToEnclosure();
	return scrollbarSet;
}

/******************************************************************************
 UpdateWindowTitle (private)

 ******************************************************************************/

void
MainDirector::UpdateWindowTitle
	(
	const JString& path
	)
{
	const JUtf8Byte* map[] =
	{
		"path", path.GetBytes()
	};
	const JString title = JGetString("WindowTitleID::MainDirector", map, sizeof(map));
	GetWindow()->SetTitle(title);
}

/******************************************************************************
 GetPath

 ******************************************************************************/

const JString&
MainDirector::GetPath()
	const
{
	assert( !itsPath.IsEmpty() );
	return itsPath;
}

/******************************************************************************
 GetRepoPath

 ******************************************************************************/

bool
MainDirector::GetRepoPath
	(
	JString* path
	)
	const
{
	if (itsRepoWidget != nullptr)
	{
		*path = (itsRepoWidget->GetRepoTree())->GetRepoPath();
		JStripTrailingDirSeparator(path);
		return true;
	}
	else
	{
		path->Clear();
		return false;
	}
}

/******************************************************************************
 RegisterActionProcess

 ******************************************************************************/

void
MainDirector::RegisterActionProcess
	(
	TabBase*		tab,
	JProcess*		p,
	const bool	refreshRepo,
	const bool	refreshStatus,
	const bool	reload
	)
{
	assert( itsActionProcess == nullptr );

	itsActionProcess = p;
	ListenTo(itsActionProcess);

	itsRefreshRepoFlag     = refreshRepo;
	itsRefreshStatusFlag   = refreshStatus;
	itsReloadOpenFilesFlag = reload;

	JIndex i;
	if (itsTabList->Find(tab, &i))
	{
		itsTabGroup->SetBusyIndex(i);
	}
}

/******************************************************************************
 ScheduleStatusRefresh

 ******************************************************************************/

void
MainDirector::ScheduleStatusRefresh()
{
	if (itsRefreshStatusTask == nullptr)
	{
		itsRefreshStatusTask = jnew RefreshStatusTask(this);
		assert( itsRefreshStatusTask != nullptr );
		itsRefreshStatusTask->Start();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
MainDirector::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsActionProcess && message.Is(JProcess::kFinished))
	{
		const auto* info =
			dynamic_cast<const JProcess::Finished*>(&message);
		assert( info != nullptr );

		itsActionProcess       = nullptr;
		bool refreshRepo   = itsRefreshRepoFlag;	// changed when refresh status
		bool refreshStatus = itsRefreshStatusFlag;	// changed when refresh status
		bool reload        = itsReloadOpenFilesFlag;

		itsTabGroup->ClearBusyIndex();
		if (info->Successful())
		{
			if (refreshRepo)
			{
				RefreshRepo();
			}

			if (refreshStatus)
			{
				RefreshStatus();
			}

			if (reload)
			{
				(GetApplication())->ReloadOpenFilesInIDE();
			}

			if (sender == itsCheckOutProcess)
			{
				itsCheckOutProcess = nullptr;
				CreateStatusTab();
			}
		}
		else
		{
			CleanUpWorkingCopy();
		}
	}

	else if (message.Is(JXCardFile::kCardRemoved))
	{
		const auto* info =
			dynamic_cast<const JXCardFile::CardRemoved*>(&message);
		assert( info != nullptr );

		JIndex i;
		if (info->GetCardIndex(&i))
		{
			itsTabList->RemoveElement(i);
		}
	}

	else if (sender == GetWindow() && message.Is(JXWindow::kDeiconified))
	{
		ScheduleStatusRefresh();
	}

	else
	{
		JXWindowDirector::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveGoingAway (virtual protected)

	The given sender has been deleted.

 ******************************************************************************/

void
MainDirector::ReceiveGoingAway
	(
	JBroadcaster* sender
	)
{
	if (sender == itsActionProcess)
	{
		itsActionProcess   = nullptr;
		itsCheckOutProcess = nullptr;
		itsTabGroup->ClearBusyIndex();
		CleanUpWorkingCopy();
	}
	else
	{
		JXWindowDirector::ReceiveGoingAway(sender);
	}
}

/******************************************************************************
 UpdateFileMenu (private)

 ******************************************************************************/

void
MainDirector::UpdateFileMenu()
{
	itsFileMenu->EnableItem(kOpenDirectoryCmd);
	itsFileMenu->EnableItem(kBrowseRepoCmd);
	itsFileMenu->EnableItem(kCheckOutRepoCmd);
	itsFileMenu->EnableItem(kCloseCmd);
	itsFileMenu->EnableItem(kQuitCmd);

	if (itsRepoWidget != nullptr && itsStatusWidget == nullptr)
	{
		itsFileMenu->EnableItem(kCheckOutCurrentRepoCmd);
	}

	JIndex i;
	if (itsTabGroup->GetCurrentTabIndex(&i))
	{
		JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
		(itsTabList->GetElement(i))->GetSelectedFiles(&list);

		if (!list.IsEmpty() && itsStatusWidget != nullptr)
		{
			itsFileMenu->EnableItem(kOpenFilesCmd);
			itsFileMenu->EnableItem(kShowFilesCmd);
		}

		if ((itsTabList->GetElement(i))->CanCheckOutSelection())
		{
			itsFileMenu->EnableItem(kCheckOutSelectionCmd);
		}
	}
}

/******************************************************************************
 HandleFileMenu (private)

 ******************************************************************************/

void
MainDirector::HandleFileMenu
	(
	const JIndex index
	)
{
	if (index == kOpenDirectoryCmd)
	{
		GetWDManager()->NewBrowser();
	}
	else if (index == kBrowseRepoCmd)
	{
		auto* dlog = jnew GetRepoDialog(JGetString("BrowseRepoWindowTitle::MainDirector"));
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			const JString& url = dlog->GetRepo();

			bool wasOpen;
			MainDirector* dir = GetWDManager()->OpenDirectory(url, &wasOpen);
			if (wasOpen)
			{
				dir->RefreshRepo();
				dir->itsRepoWidget->GetRepoTree()->SavePathToOpen(url);
				dir->RefreshStatus();
			}
		}
	}

	else if (index == kCheckOutRepoCmd)
	{
		auto* dlog = jnew GetRepoDialog(JGetString("CheckOutRepoWindowTitle::MainDirector"));
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			CheckOut(dlog->GetRepo());
		}
	}
	else if (index == kCheckOutCurrentRepoCmd)
	{
		CheckOut();
	}
	else if (index == kCheckOutSelectionCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			tab->CheckOutSelection();
		}
	}

	else if (index == kOpenFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			tab->OpenFiles();
		}
	}
	else if (index == kShowFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			tab->ShowFiles();
		}
	}

	else if (index == kCloseCmd)
	{
		GetWindow()->Close();
	}
	else if (index == kQuitCmd)
	{
		GetApplication()->Quit();
	}
}

/******************************************************************************
 CheckOut (static)

 ******************************************************************************/

void
MainDirector::CheckOut
	(
	const JString& url
	)
{
	MainDirector* dir;
	if (!(GetWDManager())->GetBrowserForExactURL(url, &dir))
	{
		dir = jnew MainDirector(JXGetApplication(), url);
		assert( dir != nullptr );
		dir->Activate();
	}
	dir->CheckOut();
}

/******************************************************************************
 CheckOut (private)

 ******************************************************************************/

void
MainDirector::CheckOut()
{
	if (itsStatusWidget != nullptr || !OKToStartActionProcess())
	{
		return;
	}

	JString repoPath;
	if (!GetRepoPath(&repoPath))
	{
		return;
	}

	JString path, name;
	JSplitPathAndName(repoPath, &path, &name);

	auto* dlog = JXSaveFileDialog::Create(JGetString("CheckOutDirectoryPrompt::MainDirector"), name);
	if (dlog->DoDialog())
	{
		const JString fullName = dlog->GetFullName();

		if (JFileExists(fullName))
		{
			JRemoveFile(fullName);
		}

		JString repoCmd("svn co ");
		repoCmd += JPrepArgForExec(repoPath);
		repoCmd += " ";
		repoCmd += JPrepArgForExec(fullName);
		Execute("CheckOutTab::MainDirector", repoCmd, false, true, false);

		itsPath            = fullName;
		itsCheckOutProcess = itsActionProcess;

		UpdateWindowTitle(JConvertToHomeDirShortcut(fullName));
	}
}

/******************************************************************************
 CreateStatusTab (private)

 ******************************************************************************/

void
MainDirector::CreateStatusTab()
{
	assert( itsStatusWidget == nullptr );

	JIndex index;
	const bool found = itsTabList->Find(itsRepoWidget, &index);
	assert( found );
	index++;

	JXContainer* card            = itsTabGroup->InsertTab(index, JGetString("StatusTab::MainDirector"));
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	itsStatusWidget =
		jnew StatusList(this, itsEditMenu,
						  scrollbarSet, scrollbarSet->GetScrollEnclosure(),
						  JXWidget::kHElastic, JXWidget::kVElastic,
						  0,0, 100,100);
	assert( itsStatusWidget != nullptr );

	itsTabList->InsertAtIndex(index, itsStatusWidget);
	itsStatusWidget->RefreshContent();
}

/******************************************************************************
 UpdateActionsMenu (private)

 ******************************************************************************/

void
MainDirector::UpdateActionsMenu()
{
	JIndex i;
	const bool hasTab = itsTabGroup->GetCurrentTabIndex(&i);
	if (hasTab && itsTabGroup->TabCanClose(i))
	{
		itsActionsMenu->EnableItem(kCloseTabCmd);
	}

	if (itsActionProcess == nullptr)
	{
		if (HasPath())
		{
			itsActionsMenu->EnableItem(kUpdateWorkingCopyCmd);
			itsActionsMenu->EnableItem(kCleanUpWorkingCopyCmd);
			itsActionsMenu->EnableItem(kCommitAllChangesCmd);
			itsActionsMenu->EnableItem(kRevertAllChangesCmd);
		}

		if (hasTab)
		{
			(itsTabList->GetElement(i))->UpdateActionsMenu(itsActionsMenu);
		}
	}
}

/******************************************************************************
 HandleActionsMenu (private)

 ******************************************************************************/

void
MainDirector::HandleActionsMenu
	(
	const JIndex index
	)
{
	if (index == kRefreshCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->RefreshContent();
		}
	}
	else if (index == kCloseTabCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i) && itsTabGroup->TabCanClose(i))
		{
			itsTabGroup->DeleteTab(i);
		}
	}

	else if (index == kUpdateWorkingCopyCmd)
	{
		UpdateWorkingCopy();
	}
	else if (index == kCleanUpWorkingCopyCmd)
	{
		CleanUpWorkingCopy();
	}

	else if (index == kAddSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->ScheduleForAdd();
		}
	}
	else if (index == kRemoveSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->ScheduleForRemove();
		}
	}
	else if (index == kForceRemoveSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->ForceScheduleForRemove();
		}
	}

	else if (index == kResolveSelectedConflictsCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->Resolved();
		}
	}

	else if (index == kCommitSelectedChangesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->Commit();
		}
	}
	else if (index == kCommitAllChangesCmd)
	{
		CommitAll();
	}

	else if (index == kRevertSelectedChangesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->Revert();
		}
	}
	else if (index == kRevertAllChangesCmd)
	{
		RevertAll();
	}

	else if (index == kCreateDirectoryCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->CreateDirectory();
		}
	}
	else if (index == kDuplicateSelectedItemCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->DuplicateItem();
		}
	}

	else if (index == kCreatePropertyCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->CreateProperty();
		}
	}
	else if (index == kRemoveSelectedPropertiesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i) &&
			JGetUserNotification()->AskUserNo(JGetString("WarnRemoveProperties::MainDirector")))
		{
			itsTabList->GetElement(i)->SchedulePropertiesForRemove();
		}
	}
	else if (index == kIgnoreSelectionCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetElement(i)->Ignore();
		}
	}
}

/******************************************************************************
 RefreshRepo

 ******************************************************************************/

void
MainDirector::RefreshRepo()
{
	if (itsRepoWidget != nullptr)
	{
		itsRepoWidget->RefreshContent();
	}
}

/******************************************************************************
 BrowseRepo

 ******************************************************************************/

void
MainDirector::BrowseRepo
	(
	const JString& rev
	)
{
	JString repoPath;
	const bool hasRepo = GetRepoPath(&repoPath);
	assert( hasRepo );

	const JUtf8Byte* map[] =
	{
		"rev", rev.GetBytes()
	};
	const JString title = JGetString("RepoRevTab::MainDirector", map, sizeof(map));

	JXContainer* card            = itsTabGroup->AppendTab(title, true);
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	auto* widget =
		jnew RepoView(this, repoPath, rev, itsEditMenu,
						scrollbarSet, scrollbarSet->GetScrollEnclosure(),
						JXWidget::kHElastic, JXWidget::kVElastic,
						0,0, 100,100);
	assert( widget != nullptr );

	itsTabGroup->ShowTab(card);
	itsTabList->Append(widget);

	widget->RefreshContent();
}

/******************************************************************************
 RefreshStatus

 ******************************************************************************/

void
MainDirector::RefreshStatus()
{
	jdelete itsRefreshStatusTask;
	itsRefreshStatusTask = nullptr;

	if (itsStatusWidget != nullptr)
	{
		itsStatusWidget->RefreshContent();
	}
}

/******************************************************************************
 UpdateWorkingCopy

 ******************************************************************************/

void
MainDirector::UpdateWorkingCopy()
{
	if (!HasPath())
	{
		return;
	}

	JXContainer* card            = itsTabGroup->AppendTab(JGetString("UpdateTab::MainDirector"), true);
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	auto* updateWidget =
		jnew UpdateList(this, itsEditMenu,
						  scrollbarSet, scrollbarSet->GetScrollEnclosure(),
						  JXWidget::kHElastic, JXWidget::kVElastic,
						  0,0, 100,100);
	assert( updateWidget != nullptr );

	itsTabGroup->ShowTab(card);
	itsTabList->Append(updateWidget);

	updateWidget->RefreshContent();
}

/******************************************************************************
 CleanUpWorkingCopy (private)

 ******************************************************************************/

void
MainDirector::CleanUpWorkingCopy()
{
	if (HasPath())
	{
		Execute("CleanUpTab::MainDirector",
				JString("svn cleanup", JString::kNoCopy), false, true, true);
	}
}

/******************************************************************************
 Commit

 ******************************************************************************/

void
MainDirector::Commit
	(
	const JString& cmd
	)
{
	if (HasPath())
	{
		Execute("CommitTab::MainDirector", cmd, true, true, false);
	}
}

/******************************************************************************
 CommitAll (private)

 ******************************************************************************/

void
MainDirector::CommitAll()
{
	if (HasPath())
	{
		Execute("CommitAllTab::MainDirector",
				JString("svn commit", JString::kNoCopy), true, true, false);
	}
}

/******************************************************************************
 RevertAll (private)

 ******************************************************************************/

void
MainDirector::RevertAll()
{
	if (HasPath() &&
		JGetUserNotification()->AskUserNo(JGetString("WarnRevertAll::MainDirector")))
	{
		Execute("RevertAllTab::MainDirector",
				JString("svn revert -R .", JString::kNoCopy), false, true, true);
	}
}

/******************************************************************************
 Execute

 ******************************************************************************/

void
MainDirector::Execute
	(
	const JUtf8Byte*	tabStringID,
	const JString&		cmd,
	const bool		refreshRepo,
	const bool		refreshStatus,
	const bool		reloadOpenFiles
	)
{
	JXContainer* card            = itsTabGroup->AppendTab(JGetString(tabStringID), true);
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	auto* widget =
		jnew CommandLog(this, itsEditMenu, cmd, refreshRepo, refreshStatus, reloadOpenFiles,
						  scrollbarSet, scrollbarSet->GetScrollEnclosure(),
						  JXWidget::kHElastic, JXWidget::kVElastic,
						  0,0, 100,100);
	assert( widget != nullptr );

	itsTabGroup->ShowTab(card);
	itsTabList->Append(widget);

	widget->RefreshContent();
}

/******************************************************************************
 UpdateInfoMenu (private)

 ******************************************************************************/

void
MainDirector::UpdateInfoMenu()
{
	itsInfoMenu->EnableItem(kBrowseRepoRevisionCmd);

	JIndex i;
	if (itsTabGroup->GetCurrentTabIndex(&i))
	{
		itsTabList->GetElement(i)->UpdateInfoMenu(itsInfoMenu);
	}
}

/******************************************************************************
 HandleInfoMenu (private)

 ******************************************************************************/

void
MainDirector::HandleInfoMenu
	(
	const JIndex index
	)
{
	if (index == kInfoLogSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			ShowInfoLog(itsTabList->GetElement(i));
		}
	}
	else if (index == kPropSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			ShowProperties(itsTabList->GetElement(i));
		}
	}

	else if (index == kDiffEditedSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			JString rev;
			if (tab->GetBaseRevision(&rev))
			{
				tab->CompareEdited(rev);
			}
		}
	}
	else if (index == kDiffCurrentSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			JString rev;
			tab->GetBaseRevision(&rev);
			tab->CompareCurrent(rev);
		}
	}
	else if (index == kDiffPrevSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			JString rev;
			tab->GetBaseRevision(&rev);
			tab->ComparePrev(rev);
		}
	}

	else if (index == kBrowseRepoRevisionCmd)
	{
		auto* dlog =
			jnew JXGetStringDialog(
				JGetString("BrowseRepoRevWindowTitle::MainDirector"),
				JGetString("BrowseRepoRevPrompt::MainDirector"));
		assert( dlog != nullptr );
		if (dlog->DoDialog())
		{
			BrowseRepo(dlog->GetString());
		}
	}
	else if (index == kBrowseSelectedRepoRevisionCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetElement(i);
			JString rev;
			if (tab->GetBaseRevision(&rev))
			{
				BrowseRepo(rev);
			}
		}
	}
}

/******************************************************************************
 ShowInfoLog

 ******************************************************************************/

void
MainDirector::ShowInfoLog
	(
	TabBase* tab
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	tab->GetSelectedFiles(&list, true);

	const JSize count = list.GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		ShowInfoLog(*(list.GetElement(i)));
	}
}

/******************************************************************************
 ShowInfoLog

 ******************************************************************************/

void
MainDirector::ShowInfoLog
	(
	const JString& origFullName,
	const JString& rev
	)
{
	JString fullName = origFullName;
	JStripTrailingDirSeparator(&fullName);

	JString path, name;
	JSplitPathAndName(fullName, &path, &name);

	const JString tabTitle       = JGetString("LogInfoTab::MainDirector") + name;
	JXContainer* card            = itsTabGroup->AppendTab(tabTitle, true);
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	auto* widget =
		jnew InfoLog(this, itsEditMenu, fullName, rev,
					   scrollbarSet, scrollbarSet->GetScrollEnclosure(),
					   JXWidget::kHElastic, JXWidget::kVElastic,
					   0,0, 100,100);
	assert( widget != nullptr );

	itsTabGroup->ShowTab(card);
	itsTabList->Append(widget);

	widget->RefreshContent();
}

/******************************************************************************
 ShowProperties

 ******************************************************************************/

void
MainDirector::ShowProperties
	(
	TabBase* tab
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	tab->GetSelectedFiles(&list, true);

	const JSize count = list.GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		ShowProperties(*(list.GetElement(i)));
	}
}

/******************************************************************************
 ShowProperties

 ******************************************************************************/

void
MainDirector::ShowProperties
	(
	const JString& origFullName
	)
{
	JString fullName = origFullName;
	JStripTrailingDirSeparator(&fullName);

	JString path, name;
	JSplitPathAndName(fullName, &path, &name);

	const JString tabTitle       = JGetString("PropTab::MainDirector") + name;
	JXContainer* card            = itsTabGroup->AppendTab(tabTitle, true);
	JXScrollbarSet* scrollbarSet = BuildScrollbarSet(card);

	auto* widget =
		jnew PropertiesList(this, itsEditMenu, fullName,
							  scrollbarSet, scrollbarSet->GetScrollEnclosure(),
							  JXWidget::kHElastic, JXWidget::kVElastic,
							  0,0, 100,100);
	assert( widget != nullptr );

	itsTabGroup->ShowTab(card);
	itsTabList->Append(widget);

	widget->RefreshContent();
}

/******************************************************************************
 HandlePrefsMenu (private)

 ******************************************************************************/

void
MainDirector::HandlePrefsMenu
	(
	const JIndex index
	)
{
	if (index == kIntegrationPrefsCmd)
	{
		GetPrefsManager()->EditPrefs();
	}
	else if (index == kEditToolBarCmd)
	{
		itsToolBar->Edit();
	}
	else if (index == kEditBindingsCmd)
	{
		JXFSBindingManager::EditBindings();
	}
	else if (index == kWebBrowserCmd)
	{
		JXGetWebBrowser()->EditPrefs();
	}
	else if (index == kEditMacWinPrefsCmd)
	{
		JXMacWinPrefsDialog::EditPrefs();
	}

	else if (index == kSaveWindSizeCmd)
	{
		GetPrefsManager()->SaveWindowSize(kMainDirectorWindSizeID, GetWindow());
	}
}

/******************************************************************************
 HandleHelpMenu (private)

 ******************************************************************************/

void
MainDirector::HandleHelpMenu
	(
	const JIndex index
	)
{
	if (index == kAboutCmd)
	{
		GetApplication()->DisplayAbout();
	}

	else if (index == kTOCCmd)
	{
		JXGetHelpManager()->ShowTOC();
	}
	else if (index == kOverviewCmd)
	{
		JXGetHelpManager()->ShowSection("OverviewHelp");
	}
	else if (index == kThisWindowCmd)
	{
		JXGetHelpManager()->ShowSection("MainHelp");
	}

	else if (index == kChangesCmd)
	{
		JXGetHelpManager()->ShowChangeLog();
	}
	else if (index == kCreditsCmd)
	{
		JXGetHelpManager()->ShowCredits();
	}
}
