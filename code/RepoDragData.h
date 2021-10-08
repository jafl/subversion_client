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

	~RepoDragData() override;

protected:

	void	AddTypes(const Atom selectionName) override;
	bool	ConvertData(const Atom requestType, Atom* returnType,
						unsigned char** data, JSize* dataLength,
						JSize* bitsPerBlock) const override;

private:

	Atom	itsType;
	JString	itsURI;
};

#endif
