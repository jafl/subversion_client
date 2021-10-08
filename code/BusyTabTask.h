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

	~BusyTabTask() override;

	void	Perform(const Time delta, Time* maxSleepTime) override;

private:

	TabGroup*	itsTabGroup;		// owns us
};

#endif
