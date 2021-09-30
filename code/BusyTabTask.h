/******************************************************************************
 BusyTabTask.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_BusyTabTask
#define _H_BusyTabTask

#include <jx-af/jx/JXIdleTask.h>

class TabGroup;

class BusyTabTask : public JXIdleTask
{
public:

	BusyTabTask(TabGroup* tabGroup);

	virtual ~BusyTabTask();

	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	TabGroup*	itsTabGroup;		// owns us
};

#endif
