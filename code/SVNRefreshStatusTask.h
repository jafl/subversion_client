/******************************************************************************
 SVNRefreshStatusTask.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNRefreshStatusTask
#define _H_SVNRefreshStatusTask

#include <jx-af/jx/JXIdleTask.h>
#include <jx-af/jcore/JBroadcaster.h>

class SVNMainDirector;

class SVNRefreshStatusTask : public JXIdleTask, virtual public JBroadcaster
{
public:

	SVNRefreshStatusTask(SVNMainDirector* directory);

	virtual ~SVNRefreshStatusTask();

	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	SVNMainDirector*	itsDirector;	// not owned
};

#endif
