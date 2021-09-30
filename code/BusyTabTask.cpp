/******************************************************************************
 BusyTabTask.cpp

	BASE CLASS = JXIdleTask

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "BusyTabTask.h"
#include "TabGroup.h"
#include <jx-af/jcore/jAssert.h>

const JSize kAnimationPeriod = 100;		// milliseconds

/******************************************************************************
 Constructor

 ******************************************************************************/

BusyTabTask::BusyTabTask
	(
	TabGroup* tabGroup
	)
	:
	JXIdleTask(kAnimationPeriod),
	itsTabGroup(tabGroup)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

BusyTabTask::~BusyTabTask()
{
}

/******************************************************************************
 Perform

 ******************************************************************************/

void
BusyTabTask::Perform
	(
	const Time	delta,
	Time*		maxSleepTime
	)
{
	if (TimeToPerform(delta, maxSleepTime))
	{
		itsTabGroup->IncrementSpinnerIndex();
	}
}
