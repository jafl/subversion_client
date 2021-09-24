/******************************************************************************
 SVNRepoTree.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_SVNRepoTree
#define _H_SVNRepoTree

#include <JTree.h>
#include <JPtrArray-JString.h>

class SVNRepoTreeList;
class SVNRepoTreeNode;

class SVNRepoTree : public JTree
{
public:

	SVNRepoTree(SVNRepoTreeNode* root);

	virtual ~SVNRepoTree();

	const JString&	GetRepoPath() const;
	void			Update(SVNRepoTreeList* view, const bool fresh = true);
	void			ReopenIfNeeded(SVNRepoTreeNode* node);
	void			SavePathToOpen(const JString& url);

	static void	SkipSetup(std::istream& input, JFileVersion vers);
	void		ReadSetup(std::istream& input, JFileVersion vers);
	void		WriteSetup(std::ostream& output, SVNRepoTreeList* view) const;

	SVNRepoTreeNode*		GetRepoRoot();
	const SVNRepoTreeNode*	GetRepoRoot() const;

private:

	SVNRepoTreeList*	itsView;			// latest caller to Update()
	JPtrArray<JString>*	itsSavedOpenNodes;	// repo paths

private:

	void	SaveOpenNodes() const;
};

#endif
