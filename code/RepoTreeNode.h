/******************************************************************************
 RepoTreeNode.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_RepoTreeNode
#define _H_RepoTreeNode

#include "jx-af/jcore/JNamedTreeNode.h"
#include <jx-af/jcore/JError.h>
#include <ace/LSOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <jx-af/jcore/JMessageProtocol.h>	// template; requires ace includes

class JProcess;
class RepoTree;

class RepoTreeNode : public JNamedTreeNode
{
public:

	using RecordLink = JMessageProtocol<ACE_LSOCK_STREAM>;

public:

	enum Type
	{
		kFile,
		kDirectory,
		kError,
		kBusy
	};

public:

	RepoTreeNode(JTree* tree, const JString& repoPath,
					const JString& repoRevision,
					const JString& name, const Type type,
					const JIndex revision, const time_t modTime,
					const JString& author, const JSize size);

	~RepoTreeNode() override;

	const JString&	GetRepoPath() const;
	bool			GetRepoRevision(JString* rev) const;
	Type			GetType() const;
	JIndex			GetRevision() const;
	time_t			GetModTime() const;
	JString			GetAgeString() const;
	const JString&	GetAuthor() const;
	JSize			GetFileSize() const;

	void	Update();
	JError	Rename(const JString& newName, const bool sort = true);

	RepoTree*			GetRepoTree();
	const RepoTree*		GetRepoTree() const;

	RepoTreeNode*		GetRepoParent();
	const RepoTreeNode*	GetRepoParent() const;
	bool				GetRepoParent(RepoTreeNode** parent);
	bool				GetRepoParent(const RepoTreeNode** parent) const;

	RepoTreeNode*		GetRepoChild(const JIndex index);
	const RepoTreeNode*	GetRepoChild(const JIndex index) const;

protected:

	bool	OKToOpen() const override;

	void	Receive(JBroadcaster* sender, const Message& message) override;
	void	ReceiveGoingAway(JBroadcaster* sender) override;

private:

	JString	itsRepoPath;
	JString	itsRepoRevision;
	bool	itsNeedUpdateFlag;

	Type	itsType;
	JIndex	itsRevision;
	time_t	itsModTime;
	JString	itsAuthor;
	JSize	itsFileSize;	// only relevant for files

	JProcess*			itsProcess;		// can be nullptr
	JString				itsResponseFullName;
	RecordLink*			itsErrorLink;	// can be nullptr
	JPtrArray<JString>*	itsErrorList;	// cache while process is running

private:

	void	SetConnection(const int errFD);
	void	DeleteLinks();
	void	ReceiveErrorLine();
	void	ParseResponse();
	void	DisplayErrors();
};


/******************************************************************************
 GetRepoPath

 ******************************************************************************/

inline const JString&
RepoTreeNode::GetRepoPath()
	const
{
	return itsRepoPath;
}

/******************************************************************************
 GetRepoRevision

 ******************************************************************************/

inline bool
RepoTreeNode::GetRepoRevision
	(
	JString* rev
	)
	const
{
	*rev = itsRepoRevision;
	return !itsRepoRevision.IsEmpty();
}

/******************************************************************************
 GetType

 ******************************************************************************/

inline RepoTreeNode::Type
RepoTreeNode::GetType()
	const
{
	return itsType;
}

/******************************************************************************
 GetRevision

 ******************************************************************************/

inline JIndex
RepoTreeNode::GetRevision()
	const
{
	return itsRevision;
}

/******************************************************************************
 GetModTime

 ******************************************************************************/

inline time_t
RepoTreeNode::GetModTime()
	const
{
	return itsModTime;
}

/******************************************************************************
 GetAuthor

 ******************************************************************************/

inline const JString&
RepoTreeNode::GetAuthor()
	const
{
	return itsAuthor;
}

/******************************************************************************
 GetFileSize

 ******************************************************************************/

inline JSize
RepoTreeNode::GetFileSize()
	const
{
	return itsFileSize;
}

#endif
