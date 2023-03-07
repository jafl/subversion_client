/******************************************************************************
 BeginEditingTask.cpp

	BASE CLASS = JXIdleTask, virtual JBroadcaster

	Copyright (C) 2000-08 by John Lindal.

 ******************************************************************************/

#include "BeginEditingTask.h"
#include "RepoView.h"
#include "RepoTreeList.h"
#include "RepoTreeNode.h"
#include <jx-af/jx/jXConstants.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

BeginEditingTask::BeginEditingTask
	(
	RepoView*		view,
	const JPoint&	cell
	)
	:
	JXIdleTask(kJXDoubleClickTime),
	itsView(view),
	itsNode((view->GetRepoTreeList())->GetRepoNode(cell.y))
{
	ClearWhenGoingAway(itsNode, &itsNode);
	ClearWhenGoingAway(itsNode, &(itsView->itsEditTask));
}

/******************************************************************************
 Destructor

 ******************************************************************************/

BeginEditingTask::~BeginEditingTask()
{
}

/******************************************************************************
 Perform

 ******************************************************************************/

void
BeginEditingTask::Perform()
{
	itsView->itsEditTask = nullptr;	// first action:  allows it to create another one, if necessary

	JPoint cell;
	if ((itsView->GetTableSelection()).GetSingleSelectedCell(&cell) &&
		(itsView->GetRepoTreeList())->GetRepoNode(cell.y) == itsNode)
	{
		itsView->BeginEditing(cell);
		itsView->TableScrollToCell(JPoint(itsView->GetToggleOpenColIndex(), cell.y));
	}

	jdelete this;
}

/******************************************************************************
 Perform (virtual protected)

 ******************************************************************************/

void
BeginEditingTask::Perform
	(
	const Time delta
	)
{
	Perform();
}
