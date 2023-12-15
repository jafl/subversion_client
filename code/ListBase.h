/******************************************************************************
 ListBase.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_ListBase
#define _H_ListBase

#include <jx-af/jx/JXStringList.h>
#include "TabBase.h"
#include <ace/LSOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <jx-af/jcore/JMessageProtocol.h>	// template; requires ace includes

class JProcess;
class JXTextMenu;

class ListBase : public JXStringList, public TabBase
{
public:

	using RecordLink = JMessageProtocol<ACE_LSOCK_STREAM>;

public:

	ListBase(MainDirector* director, JXTextMenu* editMenu,
				const JString& cmd, const bool refreshRepo,
				const bool refreshStatus, const bool reload,
				const bool enableContextMenu,
				JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	~ListBase() override;

	void	UpdateActionsMenu(JXTextMenu* menu) override;
	void	UpdateInfoMenu(JXTextMenu* menu) override;
	void	RefreshContent() override;
	void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
									 const bool includeDeleted = false) override;
	void	HandleKeyPress(const JUtf8Character& c,
								   const int keySym, const JXKeyModifiers& modifiers) override;

	virtual void	OpenSelectedItems();

protected:

	JPtrArray<JString>*	GetLineList();

	void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers) override;

	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
							const JSize clickCount,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers) override;
	void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers) override;

	virtual bool	ShouldDisplayLine(JString* line) const;
	virtual void	StyleLine(const JIndex index, const JString& line,
							  const JFontStyle& errorStyle,
							  const JFontStyle& addStyle,
							  const JFontStyle& removeStyle) = 0;
	virtual JString	ExtractRelativePath(const JString& line) const = 0;

	virtual void	UpdateContextMenu(JXTextMenu* menu);
	virtual void	CopySelectedItems(const bool fullPath);

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

	static JString	ExtractRelativePath(const JString& line, const JUnsignedOffset offset);

private:

	const JString		itsCmd;
	bool				itsRefreshRepoFlag;
	bool				itsRefreshStatusFlag;
	bool				itsReloadOpenFilesFlag;
	JXTextMenu*			itsEditMenu;		// not owned
	const bool			itsEnableContextMenuFlag;
	JXTextMenu*			itsContextMenu;		// nullptr until first used

	JPtrArray<JString>*	itsLineList;		// displayed

	JProcess*			itsProcess;			// can be nullptr
	RecordLink*			itsMessageLink;		// can be nullptr
	RecordLink*			itsErrorLink;		// can be nullptr
	JPtrArray<JString>*	itsErrorList;		// cache while process is running
	DisplayState		itsDisplayState;
	JPtrArray<JString>*	itsSavedSelection;

	bool				itsIsDraggingFlag;

private:

	void	SetConnection(const int outFD, const int errFD);
	void	DeleteLinks();
	void	ReceiveMessageLine();
	void	ReceiveErrorLine();
	void	DisplayErrors();

	void	UpdateEditMenu();
	void	HandleEditMenu(const JIndex item);

	bool	CreateContextMenu();
	void	HandleContextMenu(const JIndex index);
};


/******************************************************************************
 GetLineList (protected)

 ******************************************************************************/

inline JPtrArray<JString>*
ListBase::GetLineList()
{
	return itsLineList;
}

#endif
