/******************************************************************************
 RepoDragData.cpp

	For dragging a window to a dock.

	BASE CLASS = JXSelectionData

	Copyright (C) 2002 by John Lindal.

 ******************************************************************************/

#include "RepoDragData.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

RepoDragData::RepoDragData
	(
	JXDisplay*		display,
	const Atom		type,
	const JString&	uri
	)
	:
	JXSelectionData(display),
	itsType(type),
	itsURI(uri)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

RepoDragData::~RepoDragData()
{
}

/******************************************************************************
 AddTypes (virtual protected)

 ******************************************************************************/

void
RepoDragData::AddTypes
	(
	const Atom selectionName
	)
{
	AddType(itsType);
}

/******************************************************************************
 ConvertData (virtual protected)

 ******************************************************************************/

bool
RepoDragData::ConvertData
	(
	const Atom		requestType,
	Atom*			returnType,
	unsigned char**	data,
	JSize*			dataLength,
	JSize*			bitsPerBlock
	)
	const
{
	if (requestType == itsType)
	{
		*returnType   = XA_STRING;
		*bitsPerBlock = 8;
		*dataLength   = itsURI.GetByteCount();
		*data         = jnew_allow_null unsigned char[ *dataLength ];
		if (*data != nullptr)
		{
			memcpy(*data, itsURI.GetRawBytes(), *dataLength);
			return true;
		}
	}

	*bitsPerBlock = 8;
	*data         = nullptr;
	*dataLength   = 0;
	*returnType   = None;
	return false;
}
