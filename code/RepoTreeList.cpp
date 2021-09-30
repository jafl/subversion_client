/******************************************************************************
 RepoTreeList.cpp

	BASE CLASS = JNamedTreeList

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "RepoTreeList.h"
#include "RepoTree.h"
#include "RepoTreeNode.h"
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

RepoTreeList::RepoTreeList
	(
	RepoTree* tree
	)
	:
	JNamedTreeList(tree)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

RepoTreeList::~RepoTreeList()
{
}

/******************************************************************************
 GetRepoTree

 ******************************************************************************/

RepoTree*
RepoTreeList::GetRepoTree()
{
	auto* tree = dynamic_cast<RepoTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

const RepoTree*
RepoTreeList::GetRepoTree()
	const
{
	const auto* tree = dynamic_cast<const RepoTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

/******************************************************************************
 GetRepoNode

 ******************************************************************************/

RepoTreeNode*
RepoTreeList::GetRepoNode
	(
	const JIndex index
	)
{
	auto* node = dynamic_cast<RepoTreeNode*>(GetNode(index));
	assert (node != nullptr);
	return node;
}

const RepoTreeNode*
RepoTreeList::GetRepoNode
	(
	const JIndex index
	)
	const
{
	const auto* node = dynamic_cast<const RepoTreeNode*>(GetNode(index));
	assert (node != nullptr);
	return node;
}
