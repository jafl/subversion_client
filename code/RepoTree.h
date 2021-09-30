/******************************************************************************
 RepoTree.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_RepoTree
#define _H_RepoTree

#include <jx-af/jcore/JTree.h>
#include <jx-af/jcore/JPtrArray-JString.h>

class RepoTreeList;
class RepoTreeNode;

class RepoTree : public JTree
{
public:

	RepoTree(RepoTreeNode* root);

	virtual ~RepoTree();

	const JString&	GetRepoPath() const;
	void			Update(RepoTreeList* view, const bool fresh = true);
	void			ReopenIfNeeded(RepoTreeNode* node);
	void			SavePathToOpen(const JString& url);

	static void	SkipSetup(std::istream& input, JFileVersion vers);
	void		ReadSetup(std::istream& input, JFileVersion vers);
	void		WriteSetup(std::ostream& output, RepoTreeList* view) const;

	RepoTreeNode*		GetRepoRoot();
	const RepoTreeNode*	GetRepoRoot() const;

private:

	RepoTreeList*	itsView;			// latest caller to Update()
	JPtrArray<JString>*	itsSavedOpenNodes;	// repo paths

private:

	void	SaveOpenNodes() const;
};

#endif
