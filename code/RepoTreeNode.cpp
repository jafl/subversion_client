/******************************************************************************
 RepoTreeNode.cpp

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#include "RepoTreeNode.h"
#include "RepoTree.h"
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/JTree.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/jFileUtil.h>
#include <jx-af/jcore/jXMLUtil.h>
#include <jx-af/jcore/jTime.h>
#include <jx-af/jcore/jMath.h>
#include <libxml/parser.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

******************************************************************************/

RepoTreeNode::RepoTreeNode
	(
	JTree*			tree,
	const JString&	repoPath,
	const JString&	repoRevision,
	const JString&	name,
	const Type		type,
	const JIndex	revision,
	const time_t	modTime,
	const JString&	author,
	const JSize		size
	)
	:
	JNamedTreeNode(tree, name, type == kDirectory),
	itsRepoPath(repoPath),
	itsRepoRevision(repoRevision),
	itsNeedUpdateFlag(true),
	itsType(type),
	itsRevision(revision),
	itsModTime(modTime),
	itsAuthor(author),
	itsFileSize(size),
	itsProcess(nullptr),
	itsErrorLink(nullptr)
{
	itsErrorList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
}

/******************************************************************************
 Destructor

******************************************************************************/

RepoTreeNode::~RepoTreeNode()
{
	DeleteLinks();

	if (itsProcess != nullptr)
	{
		StopListening(itsProcess);
		itsProcess->Kill();
	}
	jdelete itsProcess;

	if (!itsResponseFullName.IsEmpty())
	{
		JRemoveFile(itsResponseFullName);
	}

	jdelete itsErrorList;
}

/******************************************************************************
 GetAgeString

 ******************************************************************************/

JString
RepoTreeNode::GetAgeString()
	const
{
	return (itsModTime == 0 ?
			JString::empty :
			JPrintTimeInterval(JRound(difftime(time(nullptr), itsModTime))));
}

/******************************************************************************
 Rename

	If sort==false, it is the caller's responsibility to call
	(node->GetParent())->SortChildren().

 ******************************************************************************/

JError
RepoTreeNode::Rename
	(
	const JString&	newName,
	const bool	sort
	)
{
	if (newName == GetName())
	{
		return JNoError();
	}

	JString path, name;
	JSplitPathAndName(itsRepoPath, &path, &name);
	const JString newRepoPath = JCombinePathAndName(path, newName);

	JString cmd("svn move ");
	cmd += itsRepoPath;
	cmd += " ";
	cmd += newRepoPath;

	JXGetApplication()->DisplayBusyCursor();

	JSimpleProcess* p;
	JSimpleProcess::Create(&p, cmd, false);
	p->WaitUntilFinished();
	if (p->SuccessfulFinish())
	{
		SetName(newName);
		itsRepoPath = newRepoPath;
		if (itsType == kDirectory)
		{
			itsNeedUpdateFlag = true;
		}

// newName may be invalid beyond this point if text is from input field

		// must be after UpdatePath() so all JDirEntries are correct,
		// because can invoke Update()

		if (sort)
		{
			GetParent()->SortChildren();		// this method maintains the selection
		}
	}

	jdelete p;
	return JNoError();
}

/******************************************************************************
 OKToOpen (virtual protected)

 ******************************************************************************/

bool
RepoTreeNode::OKToOpen()
	const
{
	if (!JNamedTreeNode::OKToOpen())
	{
		return false;
	}

	auto* me = const_cast<RepoTreeNode*>(this);
	if (itsNeedUpdateFlag)
	{
		me->itsNeedUpdateFlag = false;
		me->Update();
	}

	return true;
}

/******************************************************************************
 Update

 ******************************************************************************/

void
RepoTreeNode::Update()
{
	if (itsProcess != nullptr)
	{
		JProcess* p = itsProcess;
		itsProcess  = nullptr;

		p->Kill();
		jdelete p;

		DeleteLinks();
	}

	if (itsType != kDirectory)
	{
		return;
	}

	JError err = JNoError();
	if (!JCreateTempFile(nullptr, nullptr, &itsResponseFullName, &err))
	{
		err.ReportIfError();
		return;
	}

	JString cmd("/bin/sh -c 'svn list");
	if (!itsRepoRevision.IsEmpty())
	{
		cmd += " -r ";
		cmd += itsRepoRevision;
	}
	cmd += " --xml ";
	cmd += JPrepArgForExec(itsRepoPath);
	cmd += " >> ";
	cmd += JPrepArgForExec(itsResponseFullName);
	cmd += "'";

	int errFD;
	err = JProcess::Create(&itsProcess, cmd,
						   kJIgnoreConnection, nullptr,
						   kJIgnoreConnection, nullptr,
						   kJCreatePipe, &errFD);
	if (err.OK())
	{
		itsProcess->ShouldDeleteWhenFinished();
		ListenTo(itsProcess);
		SetConnection(errFD);

		DeleteAllChildren();

		auto* node =
			jnew RepoTreeNode(GetTree(), JString::empty, JString::empty,
								 JGetString("BusyLabel::RepoTreeNode"),
								 kBusy, 0, 0, JString::empty, 0);
		this->Append(node);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
RepoTreeNode::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsErrorLink && message.Is(JMessageProtocolT::kMessageReady))
	{
		ReceiveErrorLine();
	}
	else
	{
		JNamedTreeNode::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveGoingAway (virtual protected)

 ******************************************************************************/

void
RepoTreeNode::ReceiveGoingAway
	(
	JBroadcaster* sender
	)
{
	if (sender == itsProcess)
	{
		itsProcess = nullptr;
		DeleteLinks();

		ParseResponse();
	}
	else
	{
		JNamedTreeNode::ReceiveGoingAway(sender);
	}
}

/******************************************************************************
 ReceiveErrorLine (private)

 ******************************************************************************/

void
RepoTreeNode::ReceiveErrorLine()
{
	assert( itsErrorLink != nullptr );

	JString line;
	const bool ok = itsErrorLink->GetNextMessage(&line);
	assert( ok );

	itsErrorList->Append(line);
}

/******************************************************************************
 ParseResponse (private)

 ******************************************************************************/

void
RepoTreeNode::ParseResponse()
{
	DeleteAllChildren();

	xmlDoc* doc = xmlReadFile(itsResponseFullName.GetBytes(), nullptr, XML_PARSE_NOBLANKS | XML_PARSE_NOCDATA);
	if (doc != nullptr)
	{
		xmlNode* root = xmlDocGetRootElement(doc);

		if (root != nullptr &&
			strcmp((char*) root->name, "lists") == 0 &&
			strcmp((char*) root->children->name, "list") == 0)
		{
			const JString repoPath = JGetXMLNodeAttr(root->children, "path");

			xmlNode* entry = root->children->children;
			JString repoPath1;
			while (entry != nullptr)
			{
				const JString type = JGetXMLNodeAttr(entry, "kind");

				JString name, author;
				JIndex rev     = 0;
				time_t modTime = 0;
				JSize size     = 0;

				xmlNode* child = JGetXMLChildNode(entry, "name");
				if (child != nullptr && child->children != nullptr &&
					child->children->type == XML_TEXT_NODE)
				{
					name = (char*) child->children->content;
				}

				child = JGetXMLChildNode(entry, "size");
				if (child != nullptr && child->children != nullptr &&
					child->children->type == XML_TEXT_NODE)
				{
					size = atol((char*) child->children->content);
				}

				child = JGetXMLChildNode(entry, "commit");
				if (child != nullptr)
				{
					rev = atol(JGetXMLNodeAttr(child, "revision").GetBytes());

					xmlNode* child2 = JGetXMLChildNode(child, "author");
					if (child2 != nullptr && child2->children != nullptr &&
						child2->children->type == XML_TEXT_NODE)
					{
						author = (char*) child2->children->content;
					}

					child2 = JGetXMLChildNode(child, "date");
					if (child2 != nullptr && child2->children != nullptr &&
						child2->children->type == XML_TEXT_NODE)
					{
						tm tm;
						char* endPtr = strptime((char*) child2->children->content,
												"%Y-%m-%dT%H:%M:%S", &tm);
						if (*endPtr == '.')
						{
							modTime = mktime(&tm) + JGetTimezoneOffset();
						}
					}
				}

				if (!name.IsEmpty())
				{
					repoPath1 = JCombinePathAndName(repoPath, name);

					auto* node =
						jnew RepoTreeNode(GetTree(), repoPath1, itsRepoRevision,
											name, type == "dir" ? kDirectory : kFile,
											rev, modTime, author, size);
					this->InsertSorted(node);
				}

				entry = entry->next;
			}

			RepoTree* tree = GetRepoTree();
			const JSize count = GetChildCount();
			for (JIndex i=1; i<=count; i++)
			{
				tree->ReopenIfNeeded(GetRepoChild(i));
			}
		}

		xmlFreeDoc(doc);
	}

	JRemoveFile(itsResponseFullName);
	itsResponseFullName.Clear();

	DisplayErrors();
}

/******************************************************************************
 DisplayErrors (private)

 ******************************************************************************/

void
RepoTreeNode::DisplayErrors()
{
	const JSize count = itsErrorList->GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		auto* node =
			jnew RepoTreeNode(GetTree(), JString::empty, JString::empty,
							  *itsErrorList->GetItem(i),
							  kError, 0, 0, JString::empty, 0);
		this->InsertAtIndex(i, node);
	}

	itsErrorList->DeleteAll();
}

/******************************************************************************
 GetRepoTree

 ******************************************************************************/

RepoTree*
RepoTreeNode::GetRepoTree()
{
	auto* tree = dynamic_cast<RepoTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

const RepoTree*
RepoTreeNode::GetRepoTree()
	const
{
	const auto* tree = dynamic_cast<const RepoTree*>(GetTree());
	assert( tree != nullptr );
	return tree;
}

/******************************************************************************
 GetRepoParent

 ******************************************************************************/

RepoTreeNode*
RepoTreeNode::GetRepoParent()
{
	JTreeNode* p = GetParent();
	auto* n      = dynamic_cast<RepoTreeNode*>(p);
	assert( n != nullptr );
	return n;
}

const RepoTreeNode*
RepoTreeNode::GetRepoParent()
	const
{
	const JTreeNode* p = GetParent();
	const auto* n      = dynamic_cast<const RepoTreeNode*>(p);
	assert( n != nullptr );
	return n;
}

bool
RepoTreeNode::GetRepoParent
	(
	RepoTreeNode** parent
	)
{
	JTreeNode* p;
	if (GetParent(&p))
	{
		*parent = dynamic_cast<RepoTreeNode*>(p);
		assert( *parent != nullptr );
		return true;
	}
	else
	{
		*parent = nullptr;
		return false;
	}
}

bool
RepoTreeNode::GetRepoParent
	(
	const RepoTreeNode** parent
	)
	const
{
	const JTreeNode* p;
	if (GetParent(&p))
	{
		*parent = dynamic_cast<const RepoTreeNode*>(p);
		assert( *parent != nullptr );
		return true;
	}
	else
	{
		*parent = nullptr;
		return false;
	}
}

/******************************************************************************
 GetRepoChild

 ******************************************************************************/

RepoTreeNode*
RepoTreeNode::GetRepoChild
	(
	const JIndex index
	)
{
	auto* node = dynamic_cast<RepoTreeNode*>(GetChild(index));
	assert( node != nullptr );
	return node;
}

const RepoTreeNode*
RepoTreeNode::GetRepoChild
	(
	const JIndex index
	)
	const
{
	const auto* node = dynamic_cast<const RepoTreeNode*>(GetChild(index));
	assert( node != nullptr );
	return node;
}

/******************************************************************************
 SetConnection (private)

 ******************************************************************************/

void
RepoTreeNode::SetConnection
	(
	const int errFD
	)
{
	itsErrorLink = new RecordLink(errFD);
	assert( itsErrorLink != nullptr );
	ListenTo(itsErrorLink);
}

/******************************************************************************
 DeleteLinks (private)

 ******************************************************************************/

void
RepoTreeNode::DeleteLinks()
{
	StopListening(itsErrorLink);
	delete itsErrorLink;
	itsErrorLink = nullptr;
}
