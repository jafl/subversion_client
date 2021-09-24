/******************************************************************************
 SVNTextBase.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNTextBase
#define _H_SVNTextBase

#include <JXStaticText.h>
#include "SVNTabBase.h"
#include <ace/LSOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <JAsynchDataReceiver.h>

class JProcess;

class SVNTextBase : public JXStaticText, public SVNTabBase
{
public:

	typedef JAsynchDataReceiver<ACE_LSOCK_STREAM>	DataLink;

public:

	SVNTextBase(SVNMainDirector* director, JXTextMenu* editMenu,
				JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	virtual ~SVNTextBase();

	virtual void	UpdateActionsMenu(JXTextMenu* menu) override;
	virtual void	UpdateInfoMenu(JXTextMenu* menu) override;
	virtual void	RefreshContent() override;

protected:

	virtual JError	StartProcess(JProcess** p, int* outFD) = 0;

	virtual void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers) override;

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;
	virtual void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	JProcess*		itsProcess;		// can be nullptr
	DataLink*		itsLink;		// can be nullptr

private:

	void	SetConnection(JProcess* p, const int fd);
	void	DeleteLink();
	void	ReceiveData(const Message& message);
};

#endif
