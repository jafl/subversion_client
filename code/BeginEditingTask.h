/******************************************************************************
 BeginEditingTask.h

	Copyright (C) 2000-08 by John Lindal.

 ******************************************************************************/

#ifndef _H_BeginEditingTask
#define _H_BeginEditingTask

#include <jx-af/jx/JXIdleTask.h>
#include <jx-af/jcore/JBroadcaster.h>

class JPoint;
class RepoView;
class RepoTreeNode;

class BeginEditingTask : public JXIdleTask, virtual public JBroadcaster
{
public:

	BeginEditingTask(RepoView* table, const JPoint& cell);

	virtual ~BeginEditingTask();

	void			Perform();
	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	RepoView*		itsView;	// owns us
	RepoTreeNode*	itsNode;	// not owned; nullptr if dead
};

#endif
