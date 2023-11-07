/******************************************************************************
 UpdateList.cpp

	BASE CLASS = ListBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "UpdateList.h"
#include "globals.h"
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/JRegex.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jAssert.h>

static const JRegex revisionPattern = "revision [0-9]+\\.$";
static const JString conflictPattern("conflicts:", JString::kNoCopy);

/******************************************************************************
 Constructor

 ******************************************************************************/

UpdateList::UpdateList
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
	ListBase(director, editMenu, JString("svn --non-interactive update", JString::kNoCopy),
				false, true, true, true,
				scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	itsHasRefreshedFlag(false)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

UpdateList::~UpdateList()
{
}

/******************************************************************************
 RefreshContent (virtual)

 ******************************************************************************/

void
UpdateList::RefreshContent()
{
	if (!itsHasRefreshedFlag)
{
		ListBase::RefreshContent();
		itsHasRefreshedFlag = true;
}
}

/******************************************************************************
 StyleLine (virtual protected)

 ******************************************************************************/

void
UpdateList::StyleLine
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

	if (c1 == 'C' || c2 == 'C')
	{
		SetStyle(index, errorStyle);
	}
	else if (c1 == 'A' && c2 != 't')
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
UpdateList::ExtractRelativePath
	(
	const JString& line
	)
	const
{
	return ListBase::ExtractRelativePath(line, 6);
}

/******************************************************************************
 ShouldDisplayLine (virtual protected)

	Return false if the line should not be displayed.

 ******************************************************************************/

bool
UpdateList::ShouldDisplayLine
	(
	JString* line
	)
	const
{
	return !line->Contains(conflictPattern);
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
UpdateList::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (message.Is(JProcess::kFinished))
	{
		JPtrArray<JString>*	list = GetLineList();
		const JSize count        = list->GetItemCount();
		for (JIndex i=1; i<=count; i++)
		{
			const JString* line = list->GetItem(i);
			if (revisionPattern.Match(*line))
			{
				list->MoveItemToIndex(i, 1);
				SetStyle(1, JFontStyle(true, false, 0, false));
				break;
			}
		}
	}

	ListBase::Receive(sender, message);
}
