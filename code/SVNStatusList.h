/******************************************************************************
 SVNStatusList.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNStatusList
#define _H_SVNStatusList

#include "SVNListBase.h"

class SVNStatusList : public SVNListBase
{

public:

	SVNStatusList(SVNMainDirector* director, JXTextMenu* editMenu,
				  JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				  const HSizingOption hSizing, const VSizingOption vSizing,
				  const JCoordinate x, const JCoordinate y,
				  const JCoordinate w, const JCoordinate h);

	virtual ~SVNStatusList();

	virtual void	UpdateActionsMenu(JXTextMenu* menu) override;
	virtual bool	Ignore() override;

protected:

	virtual void	StyleLine(const JIndex index, const JString& line,
							  const JFontStyle& errorStyle,
							  const JFontStyle& addStyle,
							  const JFontStyle& removeStyle) override;
	virtual JString	ExtractRelativePath(const JString& line) const override;

	virtual void	UpdateContextMenu(JXTextMenu* menu) override;
};

#endif
