/******************************************************************************
 ListBase.cpp

	BASE CLASS = JXStringList, TabBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "ListBase.h"
#include "MainDirector.h"
#include "menus.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXDeleteObjectTask.h>
#include <jx-af/jx/JXTextSelection.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jASCIIConstants.h>
#include <jx-af/jcore/jAssert.h>

// Context menu

static const JUtf8Byte* kContextMenuStr =
	"    Compare with edited"
	"  | Compare with current"
	"  | Compare with previous"
	"%l| Info & Log"
	"  | Properties"
	"%l| Ignore..."
	"%l| Show in file manager";

	/*
	 * Remember to keep enum in ListBase.h sync
	 */

/******************************************************************************
 Constructor

	Derived class must call RefreshContent()

 ******************************************************************************/

ListBase::ListBase
	(
	MainDirector*	director,
	JXTextMenu*			editMenu,
	const JString&		cmd,
	const bool		refreshRepo,
	const bool		refreshStatus,
	const bool		reload,
	const bool		enableContextMenu,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption hSizing,
	const VSizingOption vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXStringList(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	TabBase(director),
	itsCmd(cmd),
	itsRefreshRepoFlag(refreshRepo),
	itsRefreshStatusFlag(refreshStatus),
	itsReloadOpenFilesFlag(reload),
	itsEditMenu(editMenu),
	itsEnableContextMenuFlag(enableContextMenu),
	itsContextMenu(nullptr),
	itsProcess(nullptr),
	itsMessageLink(nullptr),
	itsErrorLink(nullptr)
{
	SetFont(JFontManager::GetDefaultMonospaceFontName(), JFontManager::GetDefaultFontSize());
	SetSelectionBehavior(true, true);

	itsLineList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsLineList != nullptr );
	itsLineList->SetCompareObject(CompareLines(this));
	SetStringList(itsLineList);

	itsErrorList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsErrorList != nullptr );

	itsSavedSelection = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsSavedSelection != nullptr );
	itsSavedSelection->SetCompareFunction(JCompareStringsCaseSensitive);

	FitToEnclosure();

	ListenTo(itsEditMenu);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

ListBase::~ListBase()
{
	if (itsProcess != nullptr)
	{
		StopListening(itsProcess);
		itsProcess->Kill();
	}
	jdelete itsProcess;

	DeleteLinks();

	JXDeleteObjectTask<JBroadcaster>::Delete(itsLineList);
	jdelete itsErrorList;
	jdelete itsSavedSelection;
}

/******************************************************************************
 RefreshContent (virtual)

 ******************************************************************************/

void
ListBase::RefreshContent()
{
	if (itsProcess != nullptr)
	{
		JProcess* p = itsProcess;
		itsProcess  = nullptr;

		p->Kill();
		jdelete p;

		DeleteLinks();
	}
	else
	{
		itsSavedSelection->CleanOut();
		JTableSelection& s = GetTableSelection();
		JTableSelectionIterator iter(&s);
		JPoint cell;
		while (iter.Next(&cell))
		{
			const JString* line = itsLineList->GetElement(cell.y);
			itsSavedSelection->InsertSorted(jnew JString(ExtractRelativePath(*line)));
		}
	}

	itsDisplayState = SaveDisplayState();
	itsLineList->CleanOut();

	int outFD, errFD;
	JError err = JNoError();
	if (GetDirector()->HasPath())
	{
		err = JProcess::Create(&itsProcess, GetPath(), itsCmd,
							   kJIgnoreConnection, nullptr,
							   kJCreatePipe, &outFD,
							   kJCreatePipe, &errFD);
	}
	else	// working with URL
	{
		err = JProcess::Create(&itsProcess, itsCmd,
							   kJIgnoreConnection, nullptr,
							   kJCreatePipe, &outFD,
							   kJCreatePipe, &errFD);
	}

	if (err.OK())
	{
		itsProcess->ShouldDeleteWhenFinished();
		ListenTo(itsProcess);
		GetDirector()->RegisterActionProcess(this, itsProcess, itsRefreshRepoFlag,
											   itsRefreshStatusFlag, itsReloadOpenFilesFlag);

		SetConnection(outFD, errFD);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
ListBase::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsMessageLink && message.Is(JMessageProtocolT::kMessageReady))
	{
		ReceiveMessageLine();
	}
	else if (sender == itsErrorLink && message.Is(JMessageProtocolT::kMessageReady))
	{
		ReceiveErrorLine();
	}

	else if (sender == itsEditMenu && message.Is(JXMenu::kNeedsUpdate))
	{
		if (HasFocus())
		{
			UpdateEditMenu();
		}
	}
	else if (sender == itsEditMenu && message.Is(JXMenu::kItemSelected))
	{
		if (HasFocus())
		{
			const auto* selection =
				dynamic_cast<const JXMenu::ItemSelected*>(&message);
			assert( selection != nullptr );
			HandleEditMenu(selection->GetIndex());
		}
	}

	else if (sender == itsContextMenu && message.Is(JXMenu::kNeedsUpdate))
	{
		UpdateContextMenu(itsContextMenu);
	}
	else if (sender == itsContextMenu && message.Is(JXMenu::kItemSelected))
	{
		const auto* selection =
			dynamic_cast<const JXMenu::ItemSelected*>(&message);
		assert( selection != nullptr );
		HandleContextMenu(selection->GetIndex());
	}

	else
	{
		JXStringList::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveGoingAway (virtual protected)

 ******************************************************************************/

void
ListBase::ReceiveGoingAway
	(
	JBroadcaster* sender
	)
{
	if (sender == itsProcess)
	{
		itsProcess = nullptr;
		DeleteLinks();

		DisplayErrors();
		if (itsDisplayState.vMax > 0)
		{
			RestoreDisplayState(itsDisplayState);
		}
	}
	else
	{
		JXStringList::ReceiveGoingAway(sender);
	}
}

/******************************************************************************
 ReceiveMessageLine (private)

 ******************************************************************************/

void
ListBase::ReceiveMessageLine()
{
	assert( itsMessageLink != nullptr );

	JString line;
	const bool ok = itsMessageLink->GetNextMessage(&line);
	assert( ok );

	if (!ShouldDisplayLine(&line))
	{
		return;
	}

	const JFontStyle red(true, false, 0, false, JColorManager::GetRedColor());
	const JFontStyle blue = JColorManager::GetBlueColor();
	const JFontStyle strike(false, false, 0, true);

	auto* temp = jnew JString(line);
	assert( temp != nullptr );

	JIndex i;
	itsLineList->InsertSorted(temp, true, &i);
	StyleLine(i, line, red, blue, strike);

	JString relPath = ExtractRelativePath(line);
	JIndex j;
	if (itsSavedSelection->SearchSorted(&relPath, JListT::kAnyMatch, &j))
	{
		(GetTableSelection()).SelectRow(i);
	}
}

/******************************************************************************
 ShouldDisplayLine (virtual protected)

	Return false if the line should not be displayed.

 ******************************************************************************/

bool
ListBase::ShouldDisplayLine
	(
	JString* line
	)
	const
{
	return true;
}

/******************************************************************************
 ReceiveErrorLine (private)

 ******************************************************************************/

void
ListBase::ReceiveErrorLine()
{
	assert( itsErrorLink != nullptr );

	JString line;
	const bool ok = itsErrorLink->GetNextMessage(&line);
	assert( ok );

	itsErrorList->Append(line);
}

/******************************************************************************
 DisplayErrors (private)

 ******************************************************************************/

void
ListBase::DisplayErrors()
{
	const JFontStyle red(true, false, 0, false, JColorManager::GetRedColor());

	const JSize count = itsErrorList->GetElementCount();
	for (JIndex i=1; i<=count; i++)
	{
		itsLineList->InsertAtIndex(i, itsErrorList->GetElement(i));
		SetStyle(i, red);
	}

	itsErrorList->RemoveAll();
}

/******************************************************************************
 UpdateEditMenu (private)

 ******************************************************************************/

void
ListBase::UpdateEditMenu()
{
	const JSize count = itsEditMenu->GetItemCount();
	const JString* id;
	for (JIndex i=1; i<=count; i++)
	{
		if (itsEditMenu->GetItemID(i, &id) &&
			(((*id == kJXCopyAction || *id == kCopyFullPathAction) &&
			  (GetTableSelection()).HasSelection()) ||
			 *id == kJXSelectAllAction))
		{
			itsEditMenu->EnableItem(i);
		}
	}
}

/******************************************************************************
 HandleEditMenu (private)

 ******************************************************************************/

void
ListBase::HandleEditMenu
	(
	const JIndex index
	)
{
	const JString* id;
	if (!itsEditMenu->GetItemID(index, &id))
	{
		return;
	}

	if (*id == kJXCopyAction)
	{
		CopySelectedItems(false);
	}
	else if (*id == kCopyFullPathAction)
	{
		CopySelectedItems(true);
	}
	else if (*id == kJXSelectAllAction)
	{
		(GetTableSelection()).SelectAll();
	}
}

/******************************************************************************
 CopySelectedItems (virtual protected)

 ******************************************************************************/

void
ListBase::CopySelectedItems
	(
	const bool fullPath
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list, true);
	if (list.IsEmpty())
	{
		return;
	}

	if (!fullPath)
	{
		const JSize count       = list.GetElementCount();
		const JString& basePath = GetPath();
		for (JIndex i=1; i<=count; i++)
		{
			JString* path = list.GetElement(i);
			*path         = JConvertToRelativePath(*path, basePath);
		}
	}

	auto* data = jnew JXTextSelection(GetDisplay(), list);
	assert( data != nullptr );

	GetSelectionManager()->SetData(kJXClipboardName, data);
}

/******************************************************************************
 AdjustCursor (virtual protected)

 ******************************************************************************/

void
ListBase::AdjustCursor
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsProcess != nullptr)
	{
		DisplayCursor(kJXBusyCursor);
	}
	else
	{
		JXWidget::AdjustCursor(pt, modifiers);
	}
}

/******************************************************************************
 HandleMouseDown

 ******************************************************************************/

void
ListBase::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s = GetTableSelection();
	itsIsDraggingFlag  = false;
	itsSavedSelection->CleanOut();

	JPoint cell;
	if (button > kJXRightButton)
	{
		ScrollForWheel(button, modifiers);
	}
	else if (!GetCell(pt, &cell))
	{
		s.ClearSelection();
	}
	else if (button == kJXLeftButton && modifiers.shift())
	{
		itsIsDraggingFlag = s.ExtendSelection(cell);
	}
	else if (button == kJXLeftButton && modifiers.control())
	{
		s.InvertCell(cell);
		if (s.IsSelected(cell))
		{
			s.SetBoat(cell);
			s.SetAnchor(cell);
		}
		else
		{
			s.ClearBoat();
			s.ClearAnchor();
		}
	}
	else if (button == kJXLeftButton)
	{
		s.ClearSelection();
		s.SetBoat(cell);
		s.SetAnchor(cell);
		s.SelectCell(cell);
		itsIsDraggingFlag = true;

		if (clickCount == 2)
		{
			OpenSelectedItems();
		}
	}
	else if (button == kJXRightButton && clickCount == 1)
	{
		if (CreateContextMenu())
		{
			if (!s.IsSelected(cell))
			{
				s.ClearSelection();
				s.SetBoat(cell);
				s.SetAnchor(cell);
				s.SelectCell(cell);
			}

			itsContextMenu->PopUp(this, pt, buttonStates, modifiers);
		}
	}
}

/******************************************************************************
 HandleMouseDrag (virtual protected)

 ******************************************************************************/

void
ListBase::HandleMouseDrag
	(
	const JPoint&			pt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsIsDraggingFlag)
	{
		ScrollForDrag(pt);

		JPoint cell;
		const bool ok = GetCell(JPinInRect(pt, GetBounds()), &cell);
		assert( ok );
		(GetTableSelection()).ExtendSelection(cell);
	}
}

/******************************************************************************
 HandleKeyPress (virtual protected)

 ******************************************************************************/

void
ListBase::HandleKeyPress
	(
	const JUtf8Character&	c,
	const int				keySym,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s          = GetTableSelection();
	const bool hadSelection = s.HasSelection();
	itsSavedSelection->CleanOut();

	if (c == kJReturnKey)
	{
		OpenSelectedItems();
	}

	// space->clear is handled by JXStringList

	else if (c == kJUpArrow || c == kJDownArrow)
	{
		ClearIncrementalSearchBuffer();
		if (!hadSelection && c == kJUpArrow && GetRowCount() > 0)
		{
			SelectSingleCell(JPoint(1, GetRowCount()));
		}
		else
		{
			HandleSelectionKeyPress(c, modifiers);
		}
	}

	else
	{
		JXStringList::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 CreateContextMenu (private)

 ******************************************************************************/

#include "svn_info_log.xpm"

bool
ListBase::CreateContextMenu()
{
	if (itsContextMenu == nullptr && itsEnableContextMenuFlag)
	{
		itsContextMenu = jnew JXTextMenu(JString::empty, this, kFixedLeft, kFixedTop, 0,0, 10,10);
		assert( itsContextMenu != nullptr );
		itsContextMenu->SetMenuItems(kContextMenuStr, "ListBase");
		itsContextMenu->SetUpdateAction(JXMenu::kDisableNone);
		itsContextMenu->SetToHiddenPopupMenu();

		itsContextMenu->SetItemImage(kInfoLogSelectedFilesCtxCmd, svn_info_log);

		ListenTo(itsContextMenu);
	}

	return itsEnableContextMenuFlag && itsContextMenu != nullptr;
}

/******************************************************************************
 UpdateContextMenu (virtual protected)

 ******************************************************************************/

void
ListBase::UpdateContextMenu
	(
	JXTextMenu* menu
	)
{
	JString rev;
	menu->SetItemEnable(kDiffEditedSelectedFilesCtxCmd, GetBaseRevision(&rev));
}

/******************************************************************************
 HandleContextMenu (private)

 ******************************************************************************/

void
ListBase::HandleContextMenu
	(
	const JIndex index
	)
{
	if (index == kDiffEditedSelectedFilesCtxCmd)
	{
		JString rev;
		if (GetBaseRevision(&rev))
		{
			CompareEdited(rev);
		}
	}
	else if (index == kDiffCurrentSelectedFilesCtxCmd)
	{
		JString rev;
		GetBaseRevision(&rev);
		CompareCurrent(rev);
	}
	else if (index == kDiffPrevSelectedFilesCtxCmd)
	{
		JString rev;
		GetBaseRevision(&rev);
		ComparePrev(rev);
	}

	else if (index == kInfoLogSelectedFilesCtxCmd)
	{
		GetDirector()->ShowInfoLog(this);
	}
	else if (index == kPropSelectedFilesCtxCmd)
	{
		GetDirector()->ShowProperties(this);
	}

	else if (index == kIgnoreSelectionCtxCmd)
	{
		Ignore();
	}

	else if (index == kShowSelectedFilesCtxCmd)
	{
		ShowFiles();
	}
}

/******************************************************************************
 UpdateActionsMenu (virtual)

 ******************************************************************************/

void
ListBase::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
	menu->EnableItem(kRefreshCmd);

	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list);
	if (!list.IsEmpty())
	{
		menu->EnableItem(kAddSelectedFilesCmd);
		menu->EnableItem(kResolveSelectedConflictsCmd);
	}

	GetSelectedFiles(&list, true);
	if (!list.IsEmpty())
	{
		menu->EnableItem(kCommitSelectedChangesCmd);
		menu->EnableItem(kRemoveSelectedFilesCmd);
		menu->EnableItem(kForceRemoveSelectedFilesCmd);
		menu->EnableItem(kRevertSelectedChangesCmd);
	}
}

/******************************************************************************
 UpdateInfoMenu (virtual)

 ******************************************************************************/

void
ListBase::UpdateInfoMenu
	(
	JXTextMenu* menu
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list, true);
	if (!list.IsEmpty())
	{
		menu->EnableItem(kInfoLogSelectedFilesCmd);
		menu->EnableItem(kPropSelectedFilesCmd);
		menu->EnableItem(kDiffCurrentSelectedFilesCmd);
		menu->EnableItem(kDiffPrevSelectedFilesCmd);

		JString rev;
		if (GetBaseRevision(&rev))
		{
			menu->EnableItem(kDiffEditedSelectedFilesCmd);
		}
	}
}

/******************************************************************************
 OpenSelectedItems (virtual)

 ******************************************************************************/

void
ListBase::OpenSelectedItems()
{
	OpenFiles();
}

/******************************************************************************
 GetSelectedFiles (virtual)

 ******************************************************************************/

void
ListBase::GetSelectedFiles
	(
	JPtrArray<JString>*	fullNameList,
	const bool		includeDeleted
	)
{
	fullNameList->CleanOut();
	fullNameList->SetCleanUpAction(JPtrArrayT::kDeleteAll);

	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	JString name, fullName;
	JPoint cell;
	const JString& basePath = GetPath();
	while (iter.Next(&cell))
	{
		const JString* line   = itsLineList->GetElement(cell.y);
		name                  = ExtractRelativePath(*line);
		const bool exists = JConvertToAbsolutePath(name, basePath, &fullName);
		if (exists || includeDeleted)
		{
			fullNameList->Append(fullName);
		}
	}
}

/******************************************************************************
 ExtractRelativePath (static)

 ******************************************************************************/

JString
ListBase::ExtractRelativePath
	(
	const JString&			line,
	const JUnsignedOffset	offset
	)
{
	JStringIterator iter(line);
	iter.SkipNext(offset);
	iter.BeginMatch();
	iter.MoveTo(kJIteratorStartAtEnd, 0);
	JString s = iter.FinishMatch().GetString();
	s.TrimWhitespace();
	return s;
}

/******************************************************************************
 CompareLines class

 ******************************************************************************/

ListBase::CompareLines::CompareLines
	(
	ListBase* widget
	)
	:
	itsWidget(widget)
{
}

ListBase::CompareLines::~CompareLines()
{
}

JListT::CompareResult
ListBase::CompareLines::Compare
	(
	JString* const & s1,
	JString* const & s2
	)
	const
{
	const JString p1 = itsWidget->ExtractRelativePath(*s1);
	const JString p2 = itsWidget->ExtractRelativePath(*s2);

	const int r = JString::Compare(p1, p2, JString::kIgnoreCase);
	if (r > 0)
	{
		return JListT::kFirstGreaterSecond;
	}
	else if (r < 0)
	{
		return JListT::kFirstLessSecond;
	}
	else
	{
		return JListT::kFirstEqualSecond;
	}
}

JElementComparison<JString*>*
ListBase::CompareLines::Copy()
	const
{
	auto* copy = jnew CompareLines(itsWidget);
	assert( copy != nullptr );
	return copy;
}

/******************************************************************************
 SetConnection (private)

 ******************************************************************************/

void
ListBase::SetConnection
	(
	const int outFD,
	const int errFD
	)
{
	itsMessageLink = new RecordLink(outFD);
	assert( itsMessageLink != nullptr );
	ListenTo(itsMessageLink);

	itsErrorLink = new RecordLink(errFD);
	assert( itsErrorLink != nullptr );
	ListenTo(itsErrorLink);
}

/******************************************************************************
 DeleteLinks (private)

 ******************************************************************************/

void
ListBase::DeleteLinks()
{
	delete itsMessageLink;
	itsMessageLink = nullptr;

	delete itsErrorLink;
	itsErrorLink = nullptr;
}