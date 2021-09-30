/******************************************************************************
 RepoView.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_RepoView
#define _H_RepoView

#include <jx-af/jx/JXNamedTreeListWidget.h>
#include "TabBase.h"

class JXTextMenu;
class JXTimerTask;
class JXGetStringDialog;
class RepoTree;
class RepoTreeNode;
class RepoTreeList;
class BeginEditingTask;
class CreateRepoDirectoryDialog;
class DuplicateRepoItemDialog;

class RepoView : public JXNamedTreeListWidget, public TabBase
{
	friend class BeginEditingTask;

public:

	RepoView(MainDirector* director,
				const JString& repoPath, const JString& repoRevision,
				JXTextMenu* editMenu,
				JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	virtual ~RepoView();

	RepoTree*		GetRepoTree() const;
	RepoTreeList*	GetRepoTreeList() const;

	virtual void	UpdateActionsMenu(JXTextMenu* menu) override;
	virtual void	UpdateInfoMenu(JXTextMenu* menu) override;
	virtual void	RefreshContent() override;
	virtual void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
									 const bool includeDeleted = false) override;
	virtual void	GetSelectedFilesForDiff(JPtrArray<JString>* fullNameList,
											JArray<JIndex>* revList) override;
	virtual bool	GetBaseRevision(JString* rev) override;
	virtual void	OpenFiles() override;
	virtual void	ShowFiles() override;
	virtual bool	ScheduleForRemove() override;
	virtual bool	CreateDirectory() override;
	virtual bool	DuplicateItem() override;

	virtual bool	CanCheckOutSelection() const override;
	virtual void	CheckOutSelection() override;

	static void	SkipSetup(std::istream& input, JFileVersion vers);
	void		ReadSetup(const bool hadSetup, std::istream& input, JFileVersion vers);
	void		WriteSetup(std::ostream& output) const;

	virtual bool	IsEditable(const JPoint& cell) const override;
	virtual void		HandleKeyPress(const JUtf8Character& c,
									   const int keySym, const JXKeyModifiers& modifiers) override;

protected:

	virtual void	AdjustToTree() override;
	virtual void	TableDrawCell(JPainter &p, const JPoint& cell, const JRect& rect) override;
	virtual bool	GetImage(const JIndex index, const JXImage** image) const override;
	virtual JSize	GetMinCellWidth(const JPoint& cell) const override;

	virtual void	HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers) override;
	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;
	virtual void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;
	virtual void	HandleMouseUp(const JPoint& pt, const JXMouseButton button,
								  const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers) override;

	virtual bool	WillAcceptDrop(const JArray<Atom>& typeList, Atom* action,
									   const JPoint& pt, const Time time,
									   const JXWidget* source) override;
	virtual void	HandleDNDEnter() override;
	virtual void	HandleDNDHere(const JPoint& pt, const JXWidget* source) override;
	virtual void	HandleDNDLeave() override;
	virtual void	HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList,
								  const Atom action, const Time time,
								  const JXWidget* source) override;

	virtual Atom	GetDNDAction(const JXContainer* target,
								  const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers) override;
	virtual void	HandleDNDResponse(const JXContainer* target,
								  const bool dropAccepted, const Atom action) override;

	virtual JXInputField*
		CreateTreeListInput(const JPoint& cell, JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h) override;

	virtual bool	ExtractInputData(const JPoint& cell) override;

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	RepoTree*		itsRepoTree;
	RepoTreeList*	itsRepoTreeList;	// not owned
	JXTimerTask*		itsRefreshTask;		// refresh every minute to update age
	JColorID			itsAltRowColor;
	JXTextMenu*			itsEditMenu;		// not owned
	JXTextMenu*			itsContextMenu;		// nullptr until first used

	CreateRepoDirectoryDialog*	itsCreateDirectoryDialog;
	DuplicateRepoItemDialog*		itsDuplicateItemDialog;

	JXGetStringDialog*	itsCopyItemDialog;
	JString				itsCopyItemSrcURI;
	RepoTreeNode*	itsCopyItemDestNode;

	// Drag-and-Drop

	enum CursorType
	{
		kDNDDirCursor,
		kDNDFileCursor
	};

	CursorType	itsDNDCursorType;
	Atom		itsDNDDataType;

	// used during dragging

	JPoint	itsStartPt;
	JPoint	itsPrevPt;
	JSize	itsLastClickCount;
	bool	itsWaitingForDragFlag;
	bool	itsClearIfNotDNDFlag;

	// delayed editing

	bool					itsWaitingToEditFlag;
	BeginEditingTask*	itsEditTask;	// nullptr unless waiting to edit
	JPoint					itsEditCell;
	RepoTreeNode*		itsSortNode;	// sort when mouse released

private:

	static RepoTreeList*	BuildTreeList(const JString& repoPath,
										  const JString& repoRevision);

	JString	GetCellString(const JPoint& cell) const;
	void	StartDragRect(const JPoint& pt, const JXMouseButton button,
						  const JXKeyModifiers& modifiers);

	void	UpdateEditMenu();
	void	HandleEditMenu(const JIndex item);

	void	CreateContextMenu();
	void	UpdateContextMenu();
	void	HandleContextMenu(const JIndex index);

	void	CopySelectedFiles(const bool fullPath);
	bool	CreateDirectory1();
	bool	DuplicateItem1();
	bool	CopyItem();
};


/******************************************************************************
 GetRepoTree

 ******************************************************************************/

inline RepoTree*
RepoView::GetRepoTree()
	const
{
	return itsRepoTree;
}

/******************************************************************************
 GetRepoTreeList

 ******************************************************************************/

inline RepoTreeList*
RepoView::GetRepoTreeList()
	const
{
	return itsRepoTreeList;
}

#endif