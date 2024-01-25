/******************************************************************************
 StatusList.cpp

	BASE CLASS = ListBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "StatusList.h"
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXTextSelection.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JSubstitute.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

static const JString kIgnoreCmd("svn propedit svn:ignore $path");

/******************************************************************************
 Constructor

 ******************************************************************************/

StatusList::StatusList
	(
	MainDirector*	director,
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
	ListBase(director, editMenu, "svn --non-interactive status",
				false, false, false, true,
				scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

StatusList::~StatusList()
{
}

/******************************************************************************
 StyleLine (virtual protected)

 ******************************************************************************/

void
StatusList::StyleLine
	(
	const JIndex		index,
	const JString&		line,
	const JFontStyle&	errorStyle,
	const JFontStyle&	addStyle,
	const JFontStyle&	removeStyle
	)
{
	JStringIterator iter(line);
	JUtf8Character c1, c2;
	iter.Next(&c1);
	iter.Next(&c2);
	iter.Invalidate();

	if (c1 == 'C' || c1 == '!' || c1 == '~' || c2 == 'C')
	{
		SetStyle(index, errorStyle);
	}
	else if (c1 == '?' || c1 == 'I')
	{
		SetStyle(index, JColorManager::GetGrayColor(50));
	}
	else if (c1 == 'A')
	{
		SetStyle(index, addStyle);
	}
	else if (c1 == 'D')
	{
		SetStyle(index, removeStyle);
	}
}

/******************************************************************************
 ExtractRelativePath (virtual protected)

 ******************************************************************************/

JString
StatusList::ExtractRelativePath
	(
	const JString& line
	)
	const
{
	return ListBase::ExtractRelativePath(line, 8);
}

/******************************************************************************
 UpdateActionsMenu (virtual)

 ******************************************************************************/

#include "MainDirector-Actions-enum.h"

void
StatusList::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
	ListBase::UpdateActionsMenu(menu);

	JTableSelection& s = GetTableSelection();
	JPoint cell;
	if (s.GetSingleSelectedCell(&cell) &&
		GetStringList().GetItem(cell.y)->GetFirstCharacter() == '?')
	{
		menu->EnableItem(kIgnoreSelectionCmd);
	}
}

/******************************************************************************
 UpdateContextMenu (virtual protected)

 ******************************************************************************/

#include "ListBase-Context-enum.h"

void
StatusList::UpdateContextMenu
	(
	JXTextMenu* menu
	)
{
	ListBase::UpdateContextMenu(menu);

	bool canIgnore = false;

	JTableSelection& s = GetTableSelection();
	JPoint cell;
	if (s.GetSingleSelectedCell(&cell) &&
		GetStringList().GetItem(cell.y)->GetFirstCharacter() == '?')
	{
		canIgnore = true;
	}

	menu->SetItemEnabled(kIgnoreSelectionCtxCmd, canIgnore);
}

/******************************************************************************
 Ignore (virtual)

 ******************************************************************************/

bool
StatusList::Ignore()
{
	JTableSelection& s = GetTableSelection();
	JPoint cell;
	if (s.GetSingleSelectedCell(&cell) &&
		GetStringList().GetItem(cell.y)->GetFirstCharacter() == '?')
	{
		JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
		GetSelectedFiles(&list);

		JString path, name;
		JSplitPathAndName(*list.GetFirstItem(), &path, &name);

		auto* data = jnew JXTextSelection(GetDisplay(), name);
		assert( data != nullptr );

		GetSelectionManager()->SetData(kJXClipboardName, data);

		JString cmd = kIgnoreCmd;
		path        = JPrepArgForExec(path);

		JSubstitute subst;
		subst.DefineVariable("path", path);
		subst.Substitute(&cmd);

		JSimpleProcess::Create(cmd, true);
	}

	return true;
}
