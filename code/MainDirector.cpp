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
#include <jx-af/jx/jXActionDefs.h>
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jWebUtil.h>
#include <jx-af/jcore/jVCSUtil.h>
#include <jx-af/jcore/jAssert.h>

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

#include "MainDirector-File.h"
#include "MainDirector-Edit.h"
#include "MainDirector-Actions.h"
#include "MainDirector-Info.h"
#include "MainDirector-Preferences.h"
#include "MainDirector-Help.h"
#include "svn_main_window_icon.xpm"

void
MainDirector::BuildWindow()
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 500,300, JString::empty);
	window->SetMinSize(200, 200);
	window->SetWMClass(JXGetApplication()->GetWMName().GetBytes(), "NPS_SVN_Client_Main_Window");

	auto* menuBar =
		jnew JXMenuBar(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 500,30);
	assert( menuBar != nullptr );

	itsToolBar =
		jnew JXToolBar(GetPrefsManager(), kMainToolBarID, menuBar, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,270);

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

	window->SetIcon(jnew JXImage(GetDisplay(), svn_main_window_icon));
	ListenTo(window);

	// menus

	itsFileMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_File"));
	itsFileMenu->SetMenuItems(kFileMenuStr);
	itsFileMenu->AttachHandlers(this,
		&MainDirector::UpdateFileMenu,
		&MainDirector::HandleFileMenu);
	ConfigureFileMenu(itsFileMenu);

	itsEditMenu = JXTEBase::StaticAppendEditMenu(menuBar, false, false, false, false, false, false, false, false);

	const JSize count = itsEditMenu->GetItemCount();
	for (JIndex i=1; i<count; i++)
	{
		const JString* id;
		if (itsEditMenu->GetItemID(i, &id) && *id == kJXCopyAction)
		{
			itsEditMenu->InsertMenuItems(i+1, kEditMenuStr);
			ConfigureEditMenu(itsEditMenu, i);
			break;
		}
	}

	itsActionsMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Actions"));
	itsActionsMenu->SetMenuItems(kActionsMenuStr);
	itsActionsMenu->AttachHandlers(this,
		&MainDirector::UpdateActionsMenu,
		&MainDirector::HandleActionsMenu);
	ConfigureActionsMenu(itsActionsMenu);

	itsInfoMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Info"));
	itsInfoMenu->SetMenuItems(kInfoMenuStr);
	itsInfoMenu->AttachHandlers(this,
		&MainDirector::UpdateInfoMenu,
		&MainDirector::HandleInfoMenu);
	ConfigureInfoMenu(itsInfoMenu);

	auto* wdMenu =
		jnew JXWDMenu(JGetString("WindowsMenuTitle::JXGlobal"), menuBar,
					 JXWidget::kFixedLeft, JXWidget::kVElastic, 0,0, 10,10);
	menuBar->AppendMenu(wdMenu);
	if (JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle)
	{
		wdMenu->SetShortcuts(JGetString("WindowsMenuShortcut::JXGlobal"));
	}

	itsPrefsMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Preferences"));
	itsPrefsMenu->SetMenuItems(kPreferencesMenuStr);
	itsPrefsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsPrefsMenu->AttachHandler(this, &MainDirector::HandlePrefsMenu);
	ConfigurePreferencesMenu(itsPrefsMenu);

	itsHelpMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Help"));
	itsHelpMenu->SetMenuItems(kHelpMenuStr);
	itsHelpMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsHelpMenu->AttachHandler(this, &MainDirector::HandleHelpMenu);
	ConfigureHelpMenu(itsHelpMenu);

	// tab group

	itsTabGroup =
		jnew TabGroup(itsToolBar->GetWidgetEnclosure(),
						JXWidget::kHElastic, JXWidget::kVElastic,
						0,0, 100,100);
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

	itsToolBar->LoadPrefs(nullptr);
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
		auto& info         = dynamic_cast<const JProcess::Finished&>(message);
		itsActionProcess   = nullptr;
		bool refreshRepo   = itsRefreshRepoFlag;	// changed when refresh status
		bool refreshStatus = itsRefreshStatusFlag;	// changed when refresh status
		bool reload        = itsReloadOpenFilesFlag;

		itsTabGroup->ClearBusyIndex();
		if (info.Successful())
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
				GetApplication()->ReloadOpenFilesInIDE();
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
		auto& info = dynamic_cast<const JXCardFile::CardRemoved&>(message);
		JIndex i;
		if (info.GetCardIndex(&i))
		{
			itsTabList->RemoveItem(i);
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
		(itsTabList->GetItem(i))->GetSelectedFiles(&list);

		if (!list.IsEmpty() && itsStatusWidget != nullptr)
		{
			itsFileMenu->EnableItem(kOpenFilesCmd);
			itsFileMenu->EnableItem(kShowFilesCmd);
		}

		if ((itsTabList->GetItem(i))->CanCheckOutSelection())
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
			TabBase* tab = itsTabList->GetItem(i);
			tab->CheckOutSelection();
		}
	}

	else if (index == kOpenFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetItem(i);
			tab->OpenFiles();
		}
	}
	else if (index == kShowFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetItem(i);
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
			(itsTabList->GetItem(i))->UpdateActionsMenu(itsActionsMenu);
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
			itsTabList->GetItem(i)->RefreshContent();
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
			itsTabList->GetItem(i)->ScheduleForAdd();
		}
	}
	else if (index == kRemoveSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->ScheduleForRemove();
		}
	}
	else if (index == kForceRemoveSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->ForceScheduleForRemove();
		}
	}

	else if (index == kResolveSelectedConflictsCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->Resolved();
		}
	}

	else if (index == kCommitSelectedChangesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->Commit();
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
			itsTabList->GetItem(i)->Revert();
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
			itsTabList->GetItem(i)->CreateDirectory();
		}
	}
	else if (index == kDuplicateSelectedItemCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->DuplicateItem();
		}
	}

	else if (index == kCreatePropertyCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->CreateProperty();
		}
	}
	else if (index == kRemoveSelectedPropertiesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i) &&
			JGetUserNotification()->AskUserNo(JGetString("WarnRemoveProperties::MainDirector")))
		{
			itsTabList->GetItem(i)->SchedulePropertiesForRemove();
		}
	}
	else if (index == kIgnoreSelectionCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			itsTabList->GetItem(i)->Ignore();
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
		Execute("CleanUpTab::MainDirector", "svn cleanup", false, true, true);
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
		Execute("CommitAllTab::MainDirector", "svn commit", true, true, false);
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
		Execute("RevertAllTab::MainDirector", "svn revert -R .", false, true, true);
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
		itsTabList->GetItem(i)->UpdateInfoMenu(itsInfoMenu);
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
			ShowInfoLog(itsTabList->GetItem(i));
		}
	}
	else if (index == kPropSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			ShowProperties(itsTabList->GetItem(i));
		}
	}

	else if (index == kDiffEditedSelectedFilesCmd)
	{
		JIndex i;
		if (itsTabGroup->GetCurrentTabIndex(&i))
		{
			TabBase* tab = itsTabList->GetItem(i);
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
			TabBase* tab = itsTabList->GetItem(i);
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
			TabBase* tab = itsTabList->GetItem(i);
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
			TabBase* tab = itsTabList->GetItem(i);
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

	const JSize count = list.GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		ShowInfoLog(*(list.GetItem(i)));
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

	const JSize count = list.GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		ShowProperties(*(list.GetItem(i)));
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
