/******************************************************************************
 StatusList.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_StatusList
#define _H_StatusList

#include "ListBase.h"

class StatusList : public ListBase
{

public:

	StatusList(MainDirector* director, JXTextMenu* editMenu,
				  JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				  const HSizingOption hSizing, const VSizingOption vSizing,
				  const JCoordinate x, const JCoordinate y,
				  const JCoordinate w, const JCoordinate h);

	virtual ~StatusList();

	void	UpdateActionsMenu(JXTextMenu* menu) override;
	bool	Ignore() override;

protected:

	void	StyleLine(const JIndex index, const JString& line,
							  const JFontStyle& errorStyle,
							  const JFontStyle& addStyle,
							  const JFontStyle& removeStyle) override;
	JString	ExtractRelativePath(const JString& line) const override;

	void	UpdateContextMenu(JXTextMenu* menu) override;
};

#endif
