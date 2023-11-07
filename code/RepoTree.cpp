/******************************************************************************
 RepoTree.cpp

	BASE CLASS = JTree

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "RepoTree.h"
#include "RepoTreeNode.h"
#include "RepoTreeList.h"
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

RepoTree::RepoTree
	(
	RepoTreeNode* root
	)
	:
	JTree(root),
	itsView(nullptr)
{
	itsSavedOpenNodes = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsSavedOpenNodes != nullptr );
	itsSavedOpenNodes->SetCompareFunction(JCompareStringsCaseSensitive);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

RepoTree::~RepoTree()
{
	jdelete itsSavedOpenNodes;
}

/******************************************************************************
 GetRepoPath

 ******************************************************************************/

const JString&
RepoTree::GetRepoPath()
	const
{
	return GetRepoRoot()->GetRepoPath();
}

/******************************************************************************
 Update

 ******************************************************************************/

void
RepoTree::Update
	(
	RepoTreeList*	view,
	const bool		fresh
	)
{
	if (fresh)
	{
		itsSavedOpenNodes->CleanOut();
	}

	itsView = view;
	SaveOpenNodes();

	GetRepoRoot()->Update();
}

/******************************************************************************
 SaveOpenNodes (private)

 ******************************************************************************/

void
RepoTree::SaveOpenNodes()
	const
{
	if (itsView != nullptr)
	{
		JPtrArray<JString> paths(JPtrArrayT::kDeleteAll);
		const JSize count = itsView->GetItemCount();
		for (JIndex i=1; i<=count; i++)
		{
			if (itsView->IsOpen(i))
			{
				auto* s = jnew JString((itsView->GetRepoNode(i))->GetRepoPath());
				assert( s != nullptr );

				itsSavedOpenNodes->InsertSorted(s);
			}
		}
	}
}

/******************************************************************************
 SavePathToOpen

 ******************************************************************************/

void
RepoTree::SavePathToOpen
	(
	const JString&origUrl
	)
{
	JString url = origUrl;
	JStripTrailingDirSeparator(&url);

	JString baseUrl = GetRepoRoot()->GetRepoPath();
	JAppendDirSeparator(&baseUrl);
	if (!url.StartsWith(baseUrl))
	{
		return;
	}
	JStripTrailingDirSeparator(&baseUrl);

	JString path, name;
	while (url.GetCharacterCount() > baseUrl.GetCharacterCount())
	{
		auto* s = jnew JString(url);
		itsSavedOpenNodes->InsertSorted(s);

		JSplitPathAndName(url, &path, &name);
		url = path;
		JStripTrailingDirSeparator(&url);
	}
}

/******************************************************************************
 ReopenIfNeeded

 ******************************************************************************/

void
RepoTree::ReopenIfNeeded
	(
	RepoTreeNode* node
	)
{
	const JString& repoPath = node->GetRepoPath();
	JIndex i;
	if (itsView != nullptr &&
		itsSavedOpenNodes->SearchSorted(const_cast<JString*>(&repoPath), JListT::kAnyMatch, &i))
	{
		itsView->Open(node);
	}
}

/******************************************************************************
 SkipSetup (static)

 ******************************************************************************/

void
RepoTree::SkipSetup
	(
	std::istream&		input,
	JFileVersion	vers
	)
{
	if (vers >= 1)
	{
		JPtrArray<JString> savedOpenNodes(JPtrArrayT::kDeleteAll);
		input >> savedOpenNodes;
	}
}

/******************************************************************************
 ReadSetup

 ******************************************************************************/

void
RepoTree::ReadSetup
	(
	std::istream&		input,
	JFileVersion	vers
	)
{
	if (vers >= 1)
	{
		input >> *itsSavedOpenNodes;
	}
}

/******************************************************************************
 WriteSetup

 ******************************************************************************/

void
RepoTree::WriteSetup
	(
	std::ostream&			output,
	RepoTreeList*	view
	)
	const
{
	itsSavedOpenNodes->CleanOut();
	const_cast<RepoTree*>(this)->itsView = view;
	SaveOpenNodes();

	output << *itsSavedOpenNodes;
}

/******************************************************************************
 GetRepoRoot

 ******************************************************************************/

RepoTreeNode*
RepoTree::GetRepoRoot()
{
	auto* root = dynamic_cast<RepoTreeNode*>(GetRoot());
	assert( root != nullptr );
	return root;
}

const RepoTreeNode*
RepoTree::GetRepoRoot()
	const
{
	const auto* root = dynamic_cast<const RepoTreeNode*>(GetRoot());
	assert( root != nullptr );
	return root;
}
