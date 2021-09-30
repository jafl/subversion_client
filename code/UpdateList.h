/******************************************************************************
 UpdateList.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_UpdateList
#define _H_UpdateList

#include "ListBase.h"

class UpdateList : public ListBase
{

public:

	UpdateList(MainDirector* director, JXTextMenu* editMenu,
				  JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				  const HSizingOption hSizing, const VSizingOption vSizing,
				  const JCoordinate x, const JCoordinate y,
				  const JCoordinate w, const JCoordinate h);

	virtual ~UpdateList();

	virtual void	RefreshContent() override;

protected:

	virtual bool	ShouldDisplayLine(JString* line) const override;
	virtual void		StyleLine(const JIndex index, const JString& line,
								  const JFontStyle& errorStyle,
								  const JFontStyle& addStyle,
								  const JFontStyle& removeStyle) override;
	virtual JString		ExtractRelativePath(const JString& line) const override;

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	bool	itsHasRefreshedFlag;
};

#endif