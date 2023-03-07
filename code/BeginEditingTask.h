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

	~BeginEditingTask() override;

	void	Perform();

protected:

	void	Perform(const Time delta) override;

private:

	RepoView*		itsView;	// owns us
	RepoTreeNode*	itsNode;	// not owned; nullptr if dead
};

#endif
