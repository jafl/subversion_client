/******************************************************************************
 TabBase.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_TabBase
#define _H_TabBase

#include <jx-af/jcore/JPtrArray-JString.h>

class JXTextMenu;
class MainDirector;

class TabBase
{

public:

	TabBase(MainDirector* director);

	virtual ~TabBase();

	MainDirector*	GetDirector();
	const JString&		GetPath() const;

	virtual void	UpdateActionsMenu(JXTextMenu* menu) = 0;
	virtual void	UpdateInfoMenu(JXTextMenu* menu) = 0;
	virtual void	RefreshContent() = 0;
	virtual void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
									 const bool includeDeleted = false) = 0;
	virtual void	GetSelectedFilesForDiff(JPtrArray<JString>* fullNameList,
											JArray<JIndex>* revList);
	virtual void	OpenFiles();
	virtual void	ShowFiles();

	virtual bool	GetBaseRevision(JString* rev);
	void			CompareEdited(const JString& rev);
	void			CompareCurrent(const JString& rev = JString::empty);
	void			ComparePrev(const JString& rev = JString::empty);

	virtual bool	ScheduleForAdd();
	virtual bool	ScheduleForRemove();
	virtual bool	ForceScheduleForRemove();
	virtual bool	Resolved();
	virtual void	Commit();
	virtual bool	Revert();
	virtual bool	CreateDirectory();
	virtual bool	DuplicateItem();
	virtual bool	CreateProperty();
	virtual bool	SchedulePropertiesForRemove();
	virtual bool	Ignore();

	virtual bool	CanCheckOutSelection() const;
	virtual void	CheckOutSelection();

private:

	MainDirector*	itsDirector;

private:

	bool	Prepare(JString* cmd, const JUtf8Byte* warnMsgID = nullptr,
					const bool includeDeleted = false);
	bool	Execute(const JString& cmd, const JUtf8Byte* warnMsgID = nullptr,
					const bool includeDeleted = false,
					const bool blocking = true);
	void	Compare(const JString& rev, const bool isPrev);
	bool	ExecuteDiff(const JString& cmd, const JString& rev, const bool isPrev);
	bool	ExecuteJCCDiff(const JString& rev, const bool isPrev);

	// not allowed

	TabBase(const TabBase&) = delete;
	TabBase& operator=(const TabBase&) = delete;
};


/******************************************************************************
 GetDirector

 ******************************************************************************/

inline MainDirector*
TabBase::GetDirector()
{
	return itsDirector;
}

#endif
