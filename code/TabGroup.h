/******************************************************************************
 TabGroup.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_TabGroup
#define _H_TabGroup

#include <jx-af/jx/JXTabGroup.h>

class JXImage;
class BusyTabTask;

class TabGroup : public JXTabGroup
{
public:

	TabGroup(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	~TabGroup() override;

	void	SetBusyIndex(const JIndex index);
	void	ClearBusyIndex();

	// called by BusyTabTask

	void	IncrementSpinnerIndex();

protected:

	void	DrawTab(const JIndex index, JXWindowPainter& p,
							const JRect& rect, const Edge edge) override;
	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	JIndex	itsBusyIndex;

	JPtrArray<JXImage>*	itsImageList;
	BusyTabTask*		itsAnimationTask;
	JIndex				itsSpinnerIndex;
};

#endif
