/******************************************************************************
 PropertiesList.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_PropertiesList
#define _H_PropertiesList

#include "ListBase.h"

class JXGetStringDialog;

class PropertiesList : public ListBase
{

public:

	PropertiesList(MainDirector* director, JXTextMenu* editMenu,
					  const JString& fullName,
					  JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
					  const HSizingOption hSizing, const VSizingOption vSizing,
					  const JCoordinate x, const JCoordinate y,
					  const JCoordinate w, const JCoordinate h);

	~PropertiesList() override;

	void	UpdateActionsMenu(JXTextMenu* menu) override;
	void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
									 const bool includeDeleted = false) override;
	void	OpenSelectedItems() override;
	bool	CreateProperty() override;
	bool	SchedulePropertiesForRemove() override;

protected:

	bool	ShouldDisplayLine(JString* line) const override;
	void	StyleLine(const JIndex index, const JString& line,
							  const JFontStyle& errorStyle,
							  const JFontStyle& addStyle,
							  const JFontStyle& removeStyle) override;
	JString	ExtractRelativePath(const JString& line) const override;
	void	CopySelectedItems(const bool fullPath) override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	const JString			itsFullName;
	JXGetStringDialog*		itsCreatePropertyDialog;
	JPtrArray<JString>*		itsRemovePropertyCmdList;
	JPtrArray<JProcess>*	itsProcessList;

private:

	static JString	GetCommand(const JString& fullName);

	bool	CreateProperty1();
	bool	RemoveNextProperty();
};

#endif
