/******************************************************************************
 RefreshStatusTask.cpp

	Since there can be only one action process, this task waits until it is
	safe to refresh the working copy status.

	BASE CLASS = JXIdleTask, virtual JBroadcaster

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "RefreshStatusTask.h"
#include "MainDirector.h"
#include <jx-af/jcore/jAssert.h>

const JSize kRetryPeriod = 500;		// milliseconds

/******************************************************************************
 Constructor

 ******************************************************************************/

RefreshStatusTask::RefreshStatusTask
	(
	MainDirector* director
	)
	:
	JXIdleTask(kRetryPeriod),
	itsDirector(director)
{
	ClearWhenGoingAway(itsDirector, &itsDirector);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

RefreshStatusTask::~RefreshStatusTask()
{
}

/******************************************************************************
 Perform (virtual protected)

 ******************************************************************************/

void
RefreshStatusTask::Perform
	(
	const Time delta
	)
{
	if (itsDirector == nullptr)
	{
		jdelete this;
	}
	else if (itsDirector->OKToStartActionProcess())
	{
		itsDirector->RefreshStatus();	// deletes us
	}
}
