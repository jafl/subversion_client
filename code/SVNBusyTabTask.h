/******************************************************************************
 SVNBusyTabTask.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNBusyTabTask
#define _H_SVNBusyTabTask

#include <jx-af/jx/JXIdleTask.h>

class SVNTabGroup;

class SVNBusyTabTask : public JXIdleTask
{
public:

	SVNBusyTabTask(SVNTabGroup* tabGroup);

	virtual ~SVNBusyTabTask();

	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	SVNTabGroup*	itsTabGroup;		// owns us
};

#endif
