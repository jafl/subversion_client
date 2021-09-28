/******************************************************************************
 SVNDuplicateRepoItemDialog.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNDuplicateRepoItemDialog
#define _H_SVNDuplicateRepoItemDialog

#include <jx-af/jx/JXGetStringDialog.h>

class SVNRepoTreeNode;

class SVNDuplicateRepoItemDialog : public JXGetStringDialog
{
public:

	SVNDuplicateRepoItemDialog(JXDirector* supervisor, const JString& windowTitle,
							   const JString& prompt, const JString& initialName,
							   SVNRepoTreeNode* srcNode);

	virtual ~SVNDuplicateRepoItemDialog();

	SVNRepoTreeNode*	GetSrcNode();

protected:

	virtual bool	OKToDeactivate();

private:

	SVNRepoTreeNode*	itsSrcNode;
};


/******************************************************************************
 GetSrcNode

 ******************************************************************************/

inline SVNRepoTreeNode*
SVNDuplicateRepoItemDialog::GetSrcNode()
{
	return itsSrcNode;
}

#endif
