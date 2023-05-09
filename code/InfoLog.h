/******************************************************************************
 InfoLog.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_InfoLog
#define _H_InfoLog

#include "TextBase.h"

class JXTextMenu;

class InfoLog : public TextBase
{
public:

	InfoLog(MainDirector* director, JXTextMenu* editMenu,
			   const JString& fullName, const JString& rev,
			   JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	~InfoLog() override;

	const JString&	GetFullName() const;

	void	UpdateInfoMenu(JXTextMenu* menu) override;
	void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
							 const bool includeDeleted = false) override;
	bool	GetBaseRevision(JString* rev) override;

protected:

	JError	StartProcess(JProcess** p, int* outFD) override;

	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

private:

	const JString	itsFullName;
	const JString	itsRevision;
	JXTextMenu*		itsContextMenu;		// nullptr until first used

private:

	void	Execute(const JString& cmd);

	void	CreateContextMenu();
	void	UpdateContextMenu();
	void	HandleContextMenu(const JIndex index);
};


/******************************************************************************
 GetFullName

 ******************************************************************************/

inline const JString&
InfoLog::GetFullName()
	const
{
	return itsFullName;
}

#endif
