/******************************************************************************
 RepoDragData.h

	Copyright (C) 2011 by John Lindal.

 ******************************************************************************/

#ifndef _H_RepoDragData
#define _H_RepoDragData

#include <jx-af/jx/JXSelectionManager.h>

class RepoDragData : public JXSelectionData
{
public:

	RepoDragData(JXDisplay* display, const Atom type, const JString& uri);

	virtual	~RepoDragData();

protected:

	virtual void	AddTypes(const Atom selectionName);
	virtual bool	ConvertData(const Atom requestType, Atom* returnType,
								unsigned char** data, JSize* dataLength,
								JSize* bitsPerBlock) const;

private:

	Atom	itsType;
	JString	itsURI;
};

#endif
