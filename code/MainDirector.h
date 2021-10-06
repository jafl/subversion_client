/******************************************************************************
 MainDirector.h

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#ifndef _H_MainDirector
#define _H_MainDirector

#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jcore/JPrefObject.h>
#include <jx-af/jcore/JString.h>

class JProcess;
class JXContainer;
class JXScrollbarSet;
class JXTextMenu;
class JXToolBar;
class JXGetStringDialog;
class TabGroup;
class TabBase;
class RepoView;
class GetRepoDialog;
class StatusList;
class RefreshStatusTask;

class MainDirector : public JXWindowDirector
{
public:

	MainDirector(JXDirector* supervisor, const JString& path);
	MainDirector(JXDirector* supervisor, std::istream& input, const JFileVersion vers);

	virtual	~MainDirector();

	const JString&	GetPath() const;	// explodes if !HasPath()
	bool		HasPath() const;
	bool		GetPath(JString* path) const;
	bool		GetRepoPath(JString* path) const;
	void			RefreshRepo();
	void			BrowseRepo(const JString& rev);
	bool		GetRepoWidget(RepoView** widget);
	void			RefreshStatus();
	void			ScheduleStatusRefresh();
	void			UpdateWorkingCopy();
	void			Commit(const JString& cmd);
	void			ShowInfoLog(TabBase* tab);
	void			ShowInfoLog(const JString& fullName,
								const JString& rev = JString::empty);
	void			ShowProperties(TabBase* tab);
	void			ShowProperties(const JString& fullName);

	bool	OKToStartActionProcess() const;
	void		RegisterActionProcess(TabBase* tab, JProcess* p,
									  const bool refreshRepo,
									  const bool refreshStatus,
									  const bool reload);

	static void	CheckOut(const JString& url);

	void	Execute(const JUtf8Byte* tabStringID, const JString& cmd,
					const bool refreshRepo, const bool refreshStatus,
					const bool reloadOpenFiles);

	void	StreamOut(std::ostream& output);

protected:

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	JString	itsPath;

	JProcess*	itsActionProcess;
	JProcess*	itsCheckOutProcess;
	bool	itsRefreshRepoFlag;
	bool	itsRefreshStatusFlag;
	bool	itsReloadOpenFilesFlag;

	TabGroup*			itsTabGroup;
	RepoView*			itsRepoWidget;		// can be nullptr
	StatusList*			itsStatusWidget;	// can be nullptr
	JPtrArray<TabBase>*	itsTabList;

	JXTextMenu*	itsFileMenu;
	JXTextMenu*	itsEditMenu;
	JXTextMenu*	itsActionsMenu;
	JXTextMenu*	itsInfoMenu;
	JXTextMenu*	itsPrefsMenu;
	JXTextMenu*	itsHelpMenu;

	GetRepoDialog*	itsBrowseRepoDialog;
	JXGetStringDialog*	itsBrowseRepoRevisionDialog;
	GetRepoDialog*	itsCheckOutRepoDialog;

	RefreshStatusTask*	itsRefreshStatusTask;

// begin JXLayout

	JXToolBar* itsToolBar;

// end JXLayout

private:

	void			MainDirectorX();
	void			BuildWindow();
	JXScrollbarSet*	BuildScrollbarSet(JXContainer* widget);
	void			UpdateWindowTitle(const JString& title);

	void	UpdateFileMenu();
	void	HandleFileMenu(const JIndex index);

	void	CheckOut();
	void	CreateStatusTab();

	void	UpdateActionsMenu();
	void	HandleActionsMenu(const JIndex index);

	void	CleanUpWorkingCopy();
	void	CommitAll();
	void	RevertAll();

	void	UpdateInfoMenu();
	void	HandleInfoMenu(const JIndex index);

	void	UpdatePrefsMenu();
	void	HandlePrefsMenu(const JIndex index);

	void	UpdateHelpMenu();
	void	HandleHelpMenu(const JIndex index);
};


/******************************************************************************
 HasPath

 ******************************************************************************/

inline bool
MainDirector::HasPath()
	const
{
	return !itsPath.IsEmpty();
}

/******************************************************************************
 GetPath

 ******************************************************************************/

inline bool
MainDirector::GetPath
	(
	JString* path
	)
	const
{
	*path = itsPath;
	return !itsPath.IsEmpty();
}

/******************************************************************************
 GetRepoWidget

 ******************************************************************************/

inline bool
MainDirector::GetRepoWidget
	(
	RepoView** widget
	)
{
	*widget = itsRepoWidget;
	return itsRepoWidget != nullptr;
}

/******************************************************************************
 OKToStartActionProcess

 ******************************************************************************/

inline bool
MainDirector::OKToStartActionProcess()
	const
{
	return itsActionProcess == nullptr;
}

#endif
