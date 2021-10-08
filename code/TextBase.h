/******************************************************************************
 TextBase.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_TextBase
#define _H_TextBase

#include <jx-af/jx/JXStaticText.h>
#include "TabBase.h"
#include <ace/LSOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <jx-af/jcore/JAsynchDataReceiver.h>

class JProcess;

class TextBase : public JXStaticText, public TabBase
{
public:

	using DataLink = JAsynchDataReceiver<ACE_LSOCK_STREAM>;

public:

	TextBase(MainDirector* director, JXTextMenu* editMenu,
				JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	~TextBase() override;

	void	UpdateActionsMenu(JXTextMenu* menu) override;
	void	UpdateInfoMenu(JXTextMenu* menu) override;
	void	RefreshContent() override;

protected:

	virtual JError	StartProcess(JProcess** p, int* outFD) = 0;

	void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers) override;

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	JProcess*	itsProcess;		// can be nullptr
	DataLink*	itsLink;		// can be nullptr

private:

	void	SetConnection(JProcess* p, const int fd);
	void	DeleteLink();
	void	ReceiveData(const Message& message);
};

#endif
