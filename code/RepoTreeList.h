/******************************************************************************
 RepoTreeList.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_RepoTreeList
#define _H_RepoTreeList

#include <jx-af/jcore/JNamedTreeList.h>

class RepoTree;
class RepoTreeNode;

class RepoTreeList : public JNamedTreeList
{
public:

	RepoTreeList(RepoTree* tree);

	~RepoTreeList() override;

	RepoTree*		GetRepoTree();
	const RepoTree*	GetRepoTree() const;

	RepoTreeNode*		GetRepoNode(const JIndex index);
	const RepoTreeNode*	GetRepoNode(const JIndex index) const;
};

#endif
