/******************************************************************************
 RefreshStatusTask.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_RefreshStatusTask
#define _H_RefreshStatusTask

#include <jx-af/jx/JXIdleTask.h>
#include <jx-af/jcore/JBroadcaster.h>

class MainDirector;

class RefreshStatusTask : public JXIdleTask, virtual public JBroadcaster
{
public:

	RefreshStatusTask(MainDirector* directory);

	~RefreshStatusTask() override;

	void	Perform(const Time delta, Time* maxSleepTime) override;

private:

	MainDirector*	itsDirector;	// not owned
};

#endif
