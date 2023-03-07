/******************************************************************************
 RepoView.cpp

	BASE CLASS = JXNamedTreeListWidget, TabBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "RepoView.h"
#include "RepoTree.h"
#include "RepoTreeNode.h"
#include "RepoTreeList.h"
#include "RepoDragData.h"
#include "MainDirector.h"
#include "BeginEditingTask.h"
#include "CreateRepoDirectoryDialog.h"
#include "DuplicateRepoItemDialog.h"
#include "menus.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXDNDManager.h>
#include <jx-af/jx/JXSelectionManager.h>
#include <jx-af/jx/JXTextSelection.h>
#include <jx-af/jx/JXWebBrowser.h>
#include <jx-af/jx/JXImageCache.h>
#include <jx-af/jx/JXDragPainter.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXTimerTask.h>
#include <jx-af/jx/JXCSFDialogBase.h>
#include <jx-af/jx/jXGlobals.h>
#include <X11/keysym.h>
#include <jx-af/jcore/JTreeList.h>
#include <jx-af/jcore/JTreeNode.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jcore/JSubstitute.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jProcessUtil.h>
#include <jx-af/jcore/jMouseUtil.h>
#include <jx-af/jcore/jASCIIConstants.h>
#include <jx-af/jcore/jAssert.h>

#include <jx-af/image/jx/jx_folder_small.xpm>
#include <jx-af/image/jx/jx_folder_selected_small.xpm>
#include <jx-af/image/jx/jx_plain_file_small.xpm>
#include <jx-af/image/jx/jx_plain_file_selected_small.xpm>
#include <svn_repo_error_small.xpm>
#include <svn_repo_error_selected_small.xpm>
#include <svn_repo_busy.xpm>
#include <svn_repo_busy_selected.xpm>

static const JUtf8Byte* kDNDAtomName = "Item::RepoDragData";

const Time kRefreshInterval    = 60000;		// 1 min (milliseconds)
const JCoordinate kMarginWidth = 6;

enum
{
	kRevColIndex = 3,
	kAgeColIndex,
	kAuthorColIndex,
	kSizeColIndex
};

const JSize kExtraColCount = 4;

// Context menu

static const JUtf8Byte* kContextMenuStr =
	"    Compare with edited"
	"  | Compare with current"
	"  | Compare with previous"
	"%l| Info & Log"
	"  | Properties"
	"%l| Check out..."
	"%l| Show in file manager";

enum
{
	kDiffEditedSelectedFilesCtxCmd = 1,
	kDiffCurrentSelectedFilesCtxCmd,
	kDiffPrevSelectedFilesCtxCmd,
	kInfoLogSelectedFilesCtxCmd,
	kPropSelectedFilesCtxCmd,
	kCheckOutSelectedDirCtxCmd,
	kShowSelectedFilesCtxCmd
};

/******************************************************************************
 Constructor

 ******************************************************************************/

RepoView::RepoView
	(
	MainDirector*	director,
	const JString&		repoPath,
	const JString&		repoRevision,
	JXTextMenu*			editMenu,
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
	JXNamedTreeListWidget(BuildTreeList(repoPath, repoRevision), scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	TabBase(director),
	itsEditMenu(editMenu),
	itsContextMenu(nullptr),
	itsEditTask(nullptr),
	itsSortNode(nullptr)
{
	itsRepoTreeList = dynamic_cast<RepoTreeList*>(GetTreeList());
	assert( itsRepoTreeList != nullptr );

	itsRepoTree = itsRepoTreeList->GetRepoTree();

	itsRefreshTask = jnew JXTimerTask(kRefreshInterval);
	assert( itsRefreshTask != nullptr );
	itsRefreshTask->Start();
	ListenTo(itsRefreshTask);

	AppendCols(kExtraColCount);
	SetSelectionBehavior(true, true);
	ShouldHilightTextOnly(true);
	WantInput(true);

	itsAltRowColor = JColorManager::GetGrayColor(95);
	itsDNDDataType = GetDisplay()->RegisterXAtom(kDNDAtomName);

	FitToEnclosure();

	ListenTo(itsEditMenu);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

RepoView::~RepoView()
{
	jdelete itsRepoTree;
	jdelete itsRefreshTask;
	jdelete itsEditTask;
}

/******************************************************************************
 BuildTreeList (static private)

 ******************************************************************************/

RepoTreeList*
RepoView::BuildTreeList
	(
	const JString&	repoPath,
	const JString&	repoRevision
	)
{
	auto* root =
		jnew RepoTreeNode(nullptr, repoPath, repoRevision, JString::empty,
							RepoTreeNode::kDirectory, 0, 0, JString::empty, 0);
	assert( root != nullptr );

	auto* tree = jnew RepoTree(root);
	assert( tree != nullptr );

	auto* list = jnew RepoTreeList(tree);
	assert( list != nullptr );

	return list;
}

/******************************************************************************
 RefreshContent (virtual)

 ******************************************************************************/

void
RepoView::RefreshContent()
{
	GetDNDManager()->CancelDND();
	CancelEditing();
	GetWindow()->EndDrag(this, JPoint(0,0),
						   GetDisplay()->GetLatestButtonStates(),
						   GetDisplay()->GetLatestKeyModifiers());

	itsRepoTree->Update(itsRepoTreeList);
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
RepoView::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	if (cell.y % 2 == 0)
	{
		const JColorID origColor = p.GetPenColor();
		p.SetPenColor(itsAltRowColor);
		p.SetFilling(true);
		p.Rect(rect);
		p.SetFilling(false);
		p.SetPenColor(origColor);
	}

	if (JIndex(cell.x) == GetToggleOpenColIndex() ||
		JIndex(cell.x) == GetNodeColIndex())
	{
		JXNamedTreeListWidget::TableDrawCell(p,cell,rect);
		return;
	}

	const JString str = GetCellString(cell);
	if (!str.IsEmpty())
	{
		p.SetFont(GetFont());

		JRect r = rect;
		if (cell.x == kRevColIndex)
		{
			r.right -= kMarginWidth;
		}
		else
		{
			r.left += kMarginWidth;
		}

		p.String(r, str,
				 cell.x == kRevColIndex ? JPainter::HAlign::kRight : JPainter::HAlign::kLeft,
				 JPainter::VAlign::kCenter);
	}
}

/******************************************************************************
 GetImage (virtual protected)

 ******************************************************************************/

bool
RepoView::GetImage
	(
	const JIndex	index,
	const JXImage** image
	)
	const
{
	JXImageCache* c         = JXGetApplication()->GetDisplay(1)->GetImageCache();
	const bool selected = (GetTableSelection()).IsSelected(index, GetNodeColIndex());

	const RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(index);
	const RepoTreeNode::Type type = node->GetType();
	if (type == RepoTreeNode::kDirectory && selected)
	{
		*image = c->GetImage(jx_folder_selected_small);
	}
	else if (type == RepoTreeNode::kDirectory)
	{
		*image = c->GetImage(jx_folder_small);
	}
	else if (type == RepoTreeNode::kError && selected)
	{
		*image = c->GetImage(svn_repo_error_selected_small);
	}
	else if (type == RepoTreeNode::kError)
	{
		*image = c->GetImage(svn_repo_error_small);
	}
	else if (type == RepoTreeNode::kBusy && selected)
	{
		*image = c->GetImage(svn_repo_busy_selected);
	}
	else if (type == RepoTreeNode::kBusy)
	{
		*image = c->GetImage(svn_repo_busy);
	}
	else if (selected)
	{
		*image = c->GetImage(jx_plain_file_selected_small);
	}
	else
	{
		*image = c->GetImage(jx_plain_file_small);
	}
	return true;
}

/******************************************************************************
 GetCellString (private)

 ******************************************************************************/

JString
RepoView::GetCellString
	(
	const JPoint& cell
	)
	const
{
	const RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(cell.y);
	const RepoTreeNode::Type type = node->GetType();
	if (type == RepoTreeNode::kError || type == RepoTreeNode::kBusy)
	{
		return JString();
	}
	else if (cell.x == kRevColIndex)
	{
		return JString((JUInt64) node->GetRevision());
	}
	else if (cell.x == kAgeColIndex)
	{
		return node->GetAgeString();
	}
	else if (cell.x == kAuthorColIndex)
	{
		return node->GetAuthor();
	}
	else if (cell.x == kSizeColIndex && type == RepoTreeNode::kFile)
	{
		return JPrintFileSize(node->GetFileSize());
	}
	else if (cell.x == kSizeColIndex && type == RepoTreeNode::kDirectory)
	{
		return JString("-", JString::kNoCopy);
	}
	else
	{
		return JString();
	}
}

/******************************************************************************
 GetMinCellWidth (virtual protected)

 ******************************************************************************/

JSize
RepoView::GetMinCellWidth
	(
	const JPoint& cell
	)
	const
{
	if (JIndex(cell.x) > GetNodeColIndex())
	{
		return 2 * kMarginWidth + GetFont().GetStringWidth(GetFontManager(), GetCellString(cell));
	}
	else
	{
		return JXNamedTreeListWidget::GetMinCellWidth(cell);
	}
}

/******************************************************************************
 AdjustToTree (virtual protected)

 ******************************************************************************/

void
RepoView::AdjustToTree()
{
	const JFontStyle error(true, false, 0, false, JColorManager::GetRedColor());
	const JFontStyle busy(JColorManager::GetGrayColor(60));

	const JSize count = GetRowCount();
	for (JIndex i=1; i<=count; i++)
	{
		const RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(i);
		const RepoTreeNode::Type type = node->GetType();
		if (type == RepoTreeNode::kError)
		{
			SetCellStyle(JPoint(GetNodeColIndex(), i), error);
		}
		else if (type == RepoTreeNode::kBusy)
		{
			SetCellStyle(JPoint(GetNodeColIndex(), i), busy);
		}
	}

	JXNamedTreeListWidget::AdjustToTree();
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
RepoView::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsEditMenu && message.Is(JXMenu::kNeedsUpdate))
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
		UpdateContextMenu();
	}
	else if (sender == itsContextMenu && message.Is(JXMenu::kItemSelected))
	{
		const auto* selection =
			dynamic_cast<const JXMenu::ItemSelected*>(&message);
		assert( selection != nullptr );
		HandleContextMenu(selection->GetIndex());
	}

	else if (sender == itsRefreshTask && message.Is(JXTimerTask::kTimerWentOff))
	{
		Refresh();
	}

	else
	{
		JXNamedTreeListWidget::Receive(sender, message);
	}
}

/******************************************************************************
 UpdateEditMenu (private)

 ******************************************************************************/

void
RepoView::UpdateEditMenu()
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
RepoView::HandleEditMenu
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
		CopySelectedFiles(false);
	}
	else if (*id == kCopyFullPathAction)
	{
		CopySelectedFiles(true);
	}
	else if (*id == kJXSelectAllAction)
	{
		(GetTableSelection()).SelectAll();
	}
}

/******************************************************************************
 CopySelectedFiles (private)

 ******************************************************************************/

void
RepoView::CopySelectedFiles
	(
	const bool fullPath
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list);
	if (list.IsEmpty())
	{
		return;
	}

	if (!fullPath)
	{
		const JSize count = list.GetElementCount();
		JString path, name;
		for (JIndex i=1; i<=count; i++)
		{
			JString* s = list.GetElement(i);
			if (JSplitPathAndName(*s, &path, &name))
			{
				*s = name;
			}
		}
	}

	auto* data = jnew JXTextSelection(GetDisplay(), list);
	assert( data != nullptr );

	GetSelectionManager()->SetData(kJXClipboardName, data);
}

/******************************************************************************
 IsEditable (virtual)

	Derived classes should override this if there are some cells that
	cannot be edited.

 ******************************************************************************/

bool
RepoView::IsEditable
	(
	const JPoint& cell
	)
	const
{
	if (JIndex(cell.x) != GetNodeColIndex())
	{
		return false;
	}

	RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(cell.y);
	RepoTreeNode::Type type = node->GetType();
	return type == RepoTreeNode::kFile ||
				 type == RepoTreeNode::kDirectory;
}

/******************************************************************************
 CreateTreeListInput (virtual protected)

 ******************************************************************************/

JXInputField*
RepoView::CreateTreeListInput
	(
	const JPoint&		cell,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
{
	JTableSelection& s = GetTableSelection();
	s.ClearSelection();
	s.SelectCell(cell);

	return JXNamedTreeListWidget::CreateTreeListInput(cell, enclosure, hSizing, vSizing, x,y, w,h);
}

/******************************************************************************
 ExtractInputData (virtual protected)

 ******************************************************************************/

bool
RepoView::ExtractInputData
	(
	const JPoint& cell
	)
{
	JXInputField* input;
	const bool ok = GetXInputField(&input);
	assert( ok );

	if (input->GetText()->IsEmpty())
	{
		return true;		// treat as cancel
	}

	RepoTreeNode* node = itsRepoTreeList->GetRepoNode(cell.y);
	const JString newName = input->GetText()->GetText();	// copy since need after input field gone
	const bool sort   = (GetDisplay()->GetLatestButtonStates()).AllOff();
	const JError err      = node->Rename(newName, sort);
	input                 = nullptr;				// nodes sorted => CancelEditing()
	if (!err.OK())
	{
		JGetStringManager()->ReportError("RenameError::RepoView", err);
	}
	else if (sort)
	{
		ScrollToNode(node);
	}
	else
	{
		itsSortNode = node->GetRepoParent();
		ListenTo(itsSortNode);	// in case it dies
	}
	return err.OK();
}

/******************************************************************************
 HandleMouseHere (virtual protected)

 ******************************************************************************/

void
RepoView::HandleMouseHere
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsEditTask != nullptr && JMouseMoved(itsStartPt, pt))
	{
		itsEditTask->Perform();
	}
}

/******************************************************************************
 HandleMouseDown

 ******************************************************************************/

void
RepoView::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s = GetTableSelection();
	ClearIncrementalSearchBuffer();

	itsStartPt = itsPrevPt = pt;
	itsLastClickCount      = 0;
	itsWaitingForDragFlag  = false;
	itsClearIfNotDNDFlag   = false;
	itsWaitingToEditFlag   = false;

	jdelete itsEditTask;
	itsEditTask	= nullptr;

	JPoint cell;
	NodePart part;
	if (!GetNode(pt, &cell, &part))
	{
		if (!ScrollForWheel(button, modifiers))
		{
			StartDragRect(pt, button, modifiers);
		}
	}
	else if (part == kToggleColumn || button > kJXRightButton)
	{
		JXNamedTreeListWidget::HandleMouseDown(pt, button, clickCount,
											   buttonStates, modifiers);
		if (!IsDraggingToggle())
		{
			StartDragRect(pt, button, modifiers);
		}
	}
	else if (part == kBeforeImage || part == kAfterText ||
			 (part == kOtherColumn && (GetCellString(cell)).IsEmpty()))
	{
		StartDragRect(pt, button, modifiers);
	}
	else if (button == kJXRightButton)
	{
		if (!s.IsSelected(cell))
		{
			s.ClearSelection();
			s.SetBoat(cell);
			s.SetAnchor(cell);
			s.SelectCell(cell);
		}

		CreateContextMenu();
		itsContextMenu->PopUp(this, pt, buttonStates, modifiers);
	}
	else if (modifiers.shift() && !modifiers.control() && !modifiers.meta())
	{
		s.InvertCell(cell.y, GetNodeColIndex());
	}
	else if (clickCount > 1)
	{
		itsWaitingForDragFlag = s.HasSelection();
		itsLastClickCount     = clickCount;		// save for HandleMouseUp()
	}
	else if (modifiers.control() && !modifiers.shift() && !modifiers.meta())
	{
		// after checking for double-click, since Ctrl inverts selection

		s.InvertCell(cell.y, GetNodeColIndex());
	}
	else
	{
		itsWaitingToEditFlag = part == kInText;
		itsEditCell          = cell;

		if (s.IsSelected(cell.y, GetNodeColIndex()))
		{
			itsClearIfNotDNDFlag = true;
		}
		else
		{
			s.ClearSelection();
			s.SelectCell(cell.y, GetNodeColIndex());
		}

		itsWaitingForDragFlag = s.GetSingleSelectedCell(&cell);
	}
}

/******************************************************************************
 StartDragRect (private)

 ******************************************************************************/

void
RepoView::StartDragRect
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXKeyModifiers&	modifiers
	)
{
	if (button == kJXLeftButton)
	{
		if (!modifiers.shift())
		{
			(GetTableSelection()).ClearSelection();
		}

		JPainter* p = CreateDragInsidePainter();
		p->Rect(JRect(pt, pt));
	}
}

/******************************************************************************
 HandleMouseDrag (virtual protected)

 ******************************************************************************/

void
RepoView::HandleMouseDrag
	(
	const JPoint&			pt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JPainter* p = nullptr;
	if (GetDragPainter(&p))
	{
		if (pt != itsPrevPt)
		{
			if (!ScrollForDrag(pt))
			{
				p->Rect(JRect(itsStartPt, itsPrevPt));
			}
			p->Rect(JRect(itsStartPt, pt));
			itsPrevPt = pt;
		}
	}
	else if (itsWaitingForDragFlag)
	{
		assert( (GetTableSelection()).HasSelection() );

		if (JMouseMoved(itsStartPt, pt))
		{
			itsWaitingForDragFlag = false;
			itsClearIfNotDNDFlag  = false;
			itsWaitingToEditFlag  = false;

			JPoint cell;
			(GetTableSelection()).GetSingleSelectedCell(&cell);

			RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(cell.y);
			RepoTreeNode::Type type = node->GetType();
			if (type == RepoTreeNode::kFile ||
				type == RepoTreeNode::kDirectory)
			{
				JString uri = node->GetRepoPath();

				JString rev;
				if (node->GetRepoRevision(&rev))
				{
					uri += "@";
					uri += rev;
				}

				auto* data = jnew RepoDragData(GetDisplay(), itsDNDDataType, uri);
				assert(data != nullptr);

				itsDNDCursorType = (type == RepoTreeNode::kDirectory ? kDNDDirCursor : kDNDFileCursor);
				BeginDND(pt, buttonStates, modifiers, data);
			}
		}
	}
	else
	{
		JXNamedTreeListWidget::HandleMouseDrag(pt,buttonStates,modifiers);
	}
}

/******************************************************************************
 HandleMouseUp

 ******************************************************************************/

void
RepoView::HandleMouseUp
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JTableSelection& s = GetTableSelection();

	JPainter* p = nullptr;
	if (GetDragPainter(&p))
	{
		const JRect selRect(itsStartPt, itsPrevPt);
		p->Rect(selRect);		// erase rectangle

		// Pin in bounds so points below bounds don't change but points
		// above bounds do.

		JPoint startCell, endCell;
		const bool startOK = GetCell(JPinInRect(itsStartPt, GetBounds()), &startCell);
		const bool endOK = GetCell(JPinInRect(itsPrevPt, GetBounds()), &endCell);

		const JIndex start = JMin(startCell.y, endCell.y);
		const JIndex end   = JMax(startCell.y, endCell.y);

		if (startOK || endOK)
		{
			for (JIndex i=start; i<=end; i++)
			{
				JRect r = GetNodeRect(i);
				if (JIntersection(selRect, r, &r))
				{
					if (modifiers.shift())
					{
						s.InvertCell(i, GetNodeColIndex());
					}
					else
					{
						s.SelectCell(i, GetNodeColIndex());
					}
				}
			}
		}

		DeleteDragPainter();
		Refresh();
	}
	else if (itsWaitingForDragFlag && itsLastClickCount > 1)
	{
		OpenFiles();
	}
	else if (itsWaitingToEditFlag)
	{
		JPoint cell;
		if (GetCell(itsStartPt, &cell))		// might update dir contents
		{
			s.ClearSelection();
			s.SelectCell(itsEditCell);

			assert( itsEditTask == nullptr );
			itsEditTask = jnew BeginEditingTask(this, itsEditCell);
			assert( itsEditTask != nullptr );
			itsEditTask->Start();
		}
	}
	else if (itsClearIfNotDNDFlag)
	{
		JPoint cell;
		if (GetCell(itsStartPt, &cell))		// might update dir contents
		{
			s.ClearSelection();
			s.SelectCell(cell.y, GetNodeColIndex());
		}
	}
	else
	{
		JXNamedTreeListWidget::HandleMouseUp(pt, button, buttonStates, modifiers);
	}

	if (itsSortNode != nullptr)
	{
		itsSortNode->SortChildren();
		itsSortNode = nullptr;
	}

	itsWaitingToEditFlag = false;
	itsClearIfNotDNDFlag = false;
}

/******************************************************************************
 WillAcceptDrop (virtual protected)

 ******************************************************************************/

bool
RepoView::WillAcceptDrop
	(
	const JArray<Atom>& typeList,
	Atom*				action,
	const JPoint&		pt,
	const Time			time,
	const JXWidget*		source
	)
{
	if (!EndEditing())
	{
		return false;
	}

	const JSize typeCount = typeList.GetElementCount();
	for (JIndex i=1; i<=typeCount; i++)
	{
		const Atom type = typeList.GetElement(i);
		if (type == itsDNDDataType)
		{
			HandleDNDHere(pt, source);
			return true;
		}
	}

	return false;
}

/******************************************************************************
 HandleDNDEnter (virtual protected)

 ******************************************************************************/

void
RepoView::HandleDNDEnter()
{
	ClearDNDTargetIndex();
}

/******************************************************************************
 HandleDNDHere (virtual protected)

 ******************************************************************************/

void
RepoView::HandleDNDHere
	(
	const JPoint&	pt,
	const JXWidget* source
	)
{
	JPoint cell;
	NodePart part;
	if (!GetNode(pt, &cell, &part) ||
		part == kToggleColumn || part == kBeforeImage || part == kAfterText)
	{
		ClearDNDTargetIndex();
	}
	else
	{
		const JTreeList* treeList = GetTreeList();

		JIndex dirIndex = cell.y;

		const JTreeNode* node = treeList->GetNode(dirIndex);
		if (!node->IsOpenable())
		{
			const JTreeNode* parent = node->GetParent();
			if (!treeList->FindNode(parent, &dirIndex))
			{
				dirIndex = 0;	// if file not in subdirectory
			}
		}

		SetDNDTargetIndex(dirIndex);
	}
}

/******************************************************************************
 HandleDNDLeave (virtual protected)

 ******************************************************************************/

void
RepoView::HandleDNDLeave()
{
	ClearDNDTargetIndex();
}

/******************************************************************************
 HandleDNDDrop (virtual protected)

 ******************************************************************************/

static const JString kCopyItemCmd("svn copy $src $dst");

void
RepoView::HandleDNDDrop
	(
	const JPoint&		pt,
	const JArray<Atom>& typeList,
	const Atom			action,
	const Time			time,
	const JXWidget*		source
	)
{
	JXDNDManager* dndMgr           = GetDNDManager();
	const Atom dndName             = dndMgr->GetDNDSelectionName();
	JXSelectionManager* selManager = GetSelectionManager();
	JIndex dndIndex                = 0;

	unsigned char* data = nullptr;
	JSize dataLength;
	Atom returnType;
	JXSelectionManager::DeleteMethod delMethod;
	if (selManager->GetData(dndName, time, itsDNDDataType,
							&returnType, &data, &dataLength, &delMethod))
	{
		if (returnType == XA_STRING)
		{
			JString srcURI((char*) data, dataLength);

			JString path, initialName;
			JStripTrailingDirSeparator(&srcURI);
			JSplitPathAndName(srcURI, &path, &initialName);

			JStringIterator iter(&initialName);
			if (iter.Next("@"))
			{
				iter.SkipPrev();
				iter.RemoveAllNext();
				JStripTrailingDirSeparator(&initialName);
			}
			iter.Invalidate();

			auto* destNode =
				GetDNDTargetIndex(&dndIndex) ?
				itsRepoTreeList->GetRepoNode(dndIndex) :
				itsRepoTree->GetRepoRoot();

			auto* dlog =
				jnew JXGetStringDialog(
					JGetString("CopyItemWindowTitle::RepoView"),
					JGetString("CopyItemPrompt::RepoView"), initialName);
			assert( dlog != nullptr );
			dlog->GetInputField()->GetText()->SetCharacterInWordFunction(JXCSFDialogBase::IsCharacterInWord);

			if (dlog->DoDialog())
			{
				const JString src = JPrepArgForExec(srcURI);

				JString dst = JCombinePathAndName(destNode->GetRepoPath(), dlog->GetString());
				dst         = JPrepArgForExec(dst);

				JSubstitute subst;
				subst.DefineVariable("src", src);
				subst.DefineVariable("dst", dst);

				JString cmd = kCopyItemCmd;
				subst.Substitute(&cmd);

				GetDirector()->Execute("CopyItemTab::MainDirector", cmd,
									   true, false, false);
			}
		}

		selManager->DeleteData(&data, delMethod);
	}

	ClearDNDTargetIndex();
	ClearIncrementalSearchBuffer();
}

/******************************************************************************
 GetDNDAction (virtual protected)

	This is called repeatedly during the drag so the drop action can be
	changed based on the current target, buttons, and modifier keys.

 ******************************************************************************/

Atom
RepoView::GetDNDAction
	(
	const JXContainer*		target,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	return GetDNDManager()->GetDNDActionCopyXAtom();
}

/******************************************************************************
 HandleDNDResponse (virtual protected)

	This is called when the target indicates whether or not it will accept
	the drop.  If !dropAccepted, the action is undefined.  If the drop target
	is not within the same application, target is nullptr.

 ******************************************************************************/

void
RepoView::HandleDNDResponse
	(
	const JXContainer*	target,
	const bool		dropAccepted,
	const Atom			action
	)
{
	JXDNDManager* dndMgr = GetDNDManager();
	JCursorIndex cursor;
	if (itsDNDCursorType == kDNDDirCursor)
	{
		cursor = dndMgr->GetDNDDirectoryCursor(dropAccepted, action);
	}
	else
	{
		cursor = dndMgr->GetDNDFileCursor(dropAccepted, action);
	}
	DisplayCursor(cursor);
}

/******************************************************************************
 HandleKeyPress (virtual protected)

 ******************************************************************************/

void
RepoView::HandleKeyPress
	(
	const JUtf8Character&	c,
	const int				keySym,
	const JXKeyModifiers&	modifiers
	)
{
	if (!(GetDisplay()->GetLatestButtonStates()).AllOff())
	{
		return;		// don't let selection change during DND
	}

	if (c == kJReturnKey || keySym == XK_F2)
	{
		ClearIncrementalSearchBuffer();

		JTableSelection& s = GetTableSelection();
		JPoint cell;
		if (IsEditing())
		{
			EndEditing();
		}
		else if (s.GetSingleSelectedCell(&cell))
		{
			BeginEditing(cell);
		}
	}

	else if ((c == kJUpArrow || c == kJDownArrow) && !IsEditing())
	{
		const bool hasSelection = (GetTableSelection()).HasSelection();
		if (!hasSelection && c == kJUpArrow && GetRowCount() > 0)
		{
			SelectSingleCell(JPoint(GetNodeColIndex(), GetRowCount()));
		}
		else if (!hasSelection && c == kJDownArrow && GetRowCount() > 0)
		{
			SelectSingleCell(JPoint(GetNodeColIndex(), 1));
		}
		else
		{
			HandleSelectionKeyPress(c, modifiers);
		}
		ClearIncrementalSearchBuffer();
	}

	else
	{
		JXNamedTreeListWidget::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 CreateContextMenu (private)

 ******************************************************************************/

#include "svn_info_log.xpm"

void
RepoView::CreateContextMenu()
{
	if (itsContextMenu == nullptr)
	{
		itsContextMenu = jnew JXTextMenu(JString::empty, this, kFixedLeft, kFixedTop, 0,0, 10,10);
		assert( itsContextMenu != nullptr );
		itsContextMenu->SetMenuItems(kContextMenuStr, "RepoView");
		itsContextMenu->SetUpdateAction(JXMenu::kDisableNone);
		itsContextMenu->SetToHiddenPopupMenu();

		itsContextMenu->SetItemImage(kInfoLogSelectedFilesCtxCmd, svn_info_log);

		ListenTo(itsContextMenu);
	}
}

/******************************************************************************
 UpdateContextMenu (private)
 
 ******************************************************************************/

void
RepoView::UpdateContextMenu()
{
	JString rev;
	itsContextMenu->SetItemEnabled(kDiffEditedSelectedFilesCtxCmd,
		GetDirector()->HasPath() && GetBaseRevision(&rev));

	itsContextMenu->SetItemEnabled(kDiffCurrentSelectedFilesCtxCmd,
		(itsRepoTree->GetRepoRoot())->GetRepoRevision(&rev));

	itsContextMenu->SetItemEnabled(kCheckOutSelectedDirCtxCmd, CanCheckOutSelection());
}

/******************************************************************************
 HandleContextMenu (private)

 ******************************************************************************/

void
RepoView::HandleContextMenu
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
		if ((itsRepoTree->GetRepoRoot())->GetRepoRevision(&rev))
		{
			CompareCurrent(rev);
		}
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

	else if (index == kCheckOutSelectedDirCtxCmd)
	{
		CheckOutSelection();
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
RepoView::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
	menu->EnableItem(kRefreshCmd);

	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list);
	if (list.IsEmpty())
	{
		menu->EnableItem(kCreateDirectoryCmd);
	}
	else
	{
		menu->EnableItem(kRemoveSelectedFilesCmd);

		if (list.GetElementCount() == 1)
		{
			menu->EnableItem(kCreateDirectoryCmd);
			menu->EnableItem(kDuplicateSelectedItemCmd);
		}
	}
}

/******************************************************************************
 UpdateInfoMenu (virtual)

 ******************************************************************************/

void
RepoView::UpdateInfoMenu
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
		menu->EnableItem(kDiffPrevSelectedFilesCmd);

		JString rev;
		if (GetDirector()->HasPath() && GetBaseRevision(&rev))
		{
			menu->EnableItem(kDiffEditedSelectedFilesCmd);
		}

		if ((itsRepoTree->GetRepoRoot())->GetRepoRevision(&rev))
		{
			menu->EnableItem(kDiffCurrentSelectedFilesCmd);
		}
	}
}

/******************************************************************************
 GetSelectedFiles (virtual)

 ******************************************************************************/

void
RepoView::GetSelectedFiles
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
	while (iter.Next(&cell))
	{
		RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(cell.y);
		RepoTreeNode::Type type = node->GetType();
		if (type == RepoTreeNode::kFile ||
			type == RepoTreeNode::kDirectory)
		{
			fullNameList->Append(node->GetRepoPath());
		}
	}
}

/******************************************************************************
 GetSelectedFilesForDiff (virtual)

 ******************************************************************************/

void
RepoView::GetSelectedFilesForDiff
	(
	JPtrArray<JString>* fullNameList,
	JArray<JIndex>*		revList
	)
{
	fullNameList->CleanOut();
	fullNameList->SetCleanUpAction(JPtrArrayT::kDeleteAll);
	revList->RemoveAll();

	JString basePath;
	const bool hasCheckout = GetDirector()->GetPath(&basePath);

	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	const JString& baseURL = itsRepoTree->GetRepoPath();
	JString path, fullName;
	JPoint cell;
	while (iter.Next(&cell))
	{
		RepoTreeNode* node      = itsRepoTreeList->GetRepoNode(cell.y);
		RepoTreeNode::Type type = node->GetType();
		if (type == RepoTreeNode::kFile ||
			type == RepoTreeNode::kDirectory)
		{
			const JString& url = node->GetRepoPath();
			if (hasCheckout && url.BeginsWith(baseURL))
			{
				JStringIterator iter(url);
				iter.SkipNext(baseURL.GetCharacterCount());
				iter.BeginMatch();
				iter.MoveTo(kJIteratorStartAtEnd, 0);
				path = iter.FinishMatch().GetString();
				iter.Invalidate();

				fullName = JCombinePathAndName(basePath, path);
				fullNameList->Append(fullName);
			}
			else if (!hasCheckout)
			{
				fullNameList->Append(url);
				revList->AppendElement(node->GetRevision());
			}
		}
	}
}

/******************************************************************************
 OpenFiles (virtual)

 ******************************************************************************/

void
RepoView::OpenFiles()
{
	ShowFiles();
}

/******************************************************************************
 ShowFiles (virtual)

 ******************************************************************************/

void
RepoView::ShowFiles()
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	JArray<JIndex> revList;
	GetSelectedFilesForDiff(&list, &revList);

	(JXGetWebBrowser())->ShowFileLocations(list);
}

/******************************************************************************
 GetBaseRevision (virtual)

 ******************************************************************************/

bool
RepoView::GetBaseRevision
	(
	JString* rev
	)
{
	if ((itsRepoTree->GetRepoRoot())->GetRepoRevision(rev))
	{
		return true;
	}
	else if (GetDirector()->HasPath())
	{
		*rev = "BASE";
		return true;
	}
	else
	{
		rev->Clear();
		return false;
	}
}

/******************************************************************************
 ScheduleForRemove (virtual)

 ******************************************************************************/

bool
RepoView::ScheduleForRemove()
{
	if (TabBase::ScheduleForRemove())
	{
		GetDirector()->RefreshRepo();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 CreateDirectory (virtual)

 ******************************************************************************/

static const JString kCreateDirCmd("svn mkdir $path");

void
RepoView::CreateDirectory()
{
	RepoTreeNode* parentNode;
	JPoint cell;
	if (GetTableSelection().GetSingleSelectedCell(&cell))
	{
		parentNode = itsRepoTreeList->GetRepoNode(cell.y);

		RepoTreeNode::Type type = parentNode->GetType();
		if (type == RepoTreeNode::kFile)
		{
			parentNode = parentNode->GetRepoParent();
		}
		else if (type != RepoTreeNode::kDirectory)
		{
			return;
		}
	}
	else
	{
		parentNode = itsRepoTree->GetRepoRoot();
	}

	auto* dlog =
		jnew CreateRepoDirectoryDialog(
			JGetString("CreateDirectoryWindowTitle::RepoView"),
			JGetString("CreateDirectoryPrompt::RepoView"), JString::empty, parentNode);
	assert( dlog != nullptr );

	if (dlog->DoDialog())
	{
		RepoTreeNode* parentNode = dlog->GetParentNode();
		const JString& newName   = dlog->GetString();

		const JString path = JPrepArgForExec(
			JCombinePathAndName(parentNode->GetRepoPath(), newName));

		JSubstitute subst;
		subst.DefineVariable("path", path);

		JString cmd = kCreateDirCmd;
		subst.Substitute(&cmd);

		GetDirector()->Execute("CreateDirectoryTab::MainDirector", cmd,
								 true, false, false);
	}
}

/******************************************************************************
 DuplicateItem (virtual)

 ******************************************************************************/

static const JString kDuplicateItemCmd("svn copy $rev $src $dst");

void
RepoView::DuplicateItem()
{
	JPoint cell;
	if (!(GetTableSelection()).GetSingleSelectedCell(&cell))
	{
		return;
	}

	RepoTreeNode* srcNode  = itsRepoTreeList->GetRepoNode(cell.y);

	JString initialName, root, suffix;
	if (JSplitRootAndSuffix(srcNode->GetName(), &root, &suffix))
	{
		root       += JGetString("DuplicateItemSuffix::RepoView");
		initialName = JCombineRootAndSuffix(root, suffix);
	}
	else
	{
		initialName = srcNode->GetName() + JGetString("DuplicateItemSuffix::RepoView");
	}

	auto* dlog =
		jnew DuplicateRepoItemDialog(
			JGetString("DuplicateItemWindowTitle::RepoView"),
			JGetString("DuplicateItemPrompt::RepoView"), initialName, srcNode);
	assert( dlog != nullptr );

	if (dlog->DoDialog())
	{
		RepoTreeNode* srcNode  = dlog->GetSrcNode();
		const JString& newName = dlog->GetString();

		JString rev;
		if (srcNode->GetRepoRevision(&rev))
		{
			rev.Prepend("-r ");
		}

		const JString src = JPrepArgForExec(srcNode->GetRepoPath());

		JString path, name;
		JSplitPathAndName(srcNode->GetRepoPath(), &path, &name);
		name = JCombinePathAndName(path, newName);

		const JString dst = JPrepArgForExec(name);

		JSubstitute subst;
		subst.DefineVariable("rev", rev);
		subst.DefineVariable("src", src);
		subst.DefineVariable("dst", dst);

		JString cmd = kDuplicateItemCmd;
		subst.Substitute(&cmd);

		GetDirector()->Execute("DuplicateItemTab::MainDirector", cmd,
							   true, false, false);
	}
}

/******************************************************************************
 CanCheckOutSelection (virtual)

 ******************************************************************************/

bool
RepoView::CanCheckOutSelection()
	const
{
	JPoint cell;
	if (GetTableSelection().GetSingleSelectedCell(&cell))
	{
		RepoTreeNode* node = itsRepoTreeList->GetRepoNode(cell.y);
		if (node->GetType() == RepoTreeNode::kDirectory)
		{
			return true;
		}
	}

	return false;
}

/******************************************************************************
 CheckOutSelection (virtual)

 ******************************************************************************/

void
RepoView::CheckOutSelection()
{
	JPoint cell;
	if ((GetTableSelection()).GetSingleSelectedCell(&cell))
	{
		RepoTreeNode* node = itsRepoTreeList->GetRepoNode(cell.y);
		if (node->GetType() == RepoTreeNode::kDirectory)
		{
			MainDirector::CheckOut(node->GetRepoPath());
		}
	}
}

/******************************************************************************
 SkipSetup (static)

 ******************************************************************************/

void
RepoView::SkipSetup
	(
	std::istream&		input,
	JFileVersion	vers
	)
{
	RepoTree::SkipSetup(input, vers);
}

/******************************************************************************
 ReadSetup

 ******************************************************************************/

void
RepoView::ReadSetup
	(
	const bool	hadSetup,
	std::istream&		input,
	JFileVersion	vers
	)
{
	if (hadSetup)
	{
		itsRepoTree->ReadSetup(input, vers);
	}

	itsRepoTree->Update(itsRepoTreeList, false);
}

/******************************************************************************
 WriteSetup

 ******************************************************************************/

void
RepoView::WriteSetup
	(
	std::ostream& output
	)
	const
{
	itsRepoTree->WriteSetup(output, itsRepoTreeList);
}
