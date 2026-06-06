/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#ifndef __APEX_STRING_H__
#define __APEX_STRING_H__

#include "PsShare.h"
#include "PsArray.h"
#include "PsString.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace physx {
namespace apex {

/**
 * ApexSimpleString - a simple string class
 */
class ApexSimpleString : public physx::Array<char>, public physx::UserAllocated
{
public:
	ApexSimpleString() : length(0), physx::Array<char>()
	{
	}

	explicit ApexSimpleString( const char* cStr ) : length(0), physx::Array<char>()
	{
		if( cStr )
		{
			length = (physx::PxU32)strlen( cStr );
			if( length > 0 )
			{
				resize( length + 1 );
				physx::string::strcpy_s( begin(), size(), cStr );
			}
		}
	}

	ApexSimpleString( const ApexSimpleString& other )
	{
		length = other.length;
		if( length > 0 )
		{
			resize( length+1 );
			physx::string::strcpy_s( begin(), capacity(), other.c_str() );
		}
		else
		{
			resize( 0 );
		}
	}

	/// \todo
	ApexSimpleString( physx::PxU32 number ) : length(0)
	{
		PX_FORCE_PARAMETER_REFERENCE(number);
	}

	ApexSimpleString& operator = ( const ApexSimpleString& other )
	{
		length = other.length;
		if( length > 0 )
		{
			resize( length + 1 );
			physx::string::strcpy_s( begin(), capacity(), other.c_str() );
		}
		else
		{
			resize( 0 );
		}
		return *this;
	}

	ApexSimpleString& operator = ( const char* cStr )
	{
		if( !cStr )
		{
			erase();
		}
		else
		{
			length = (physx::PxU32)strlen( cStr );
			if( length > 0 )
			{
				resize( length + 1 );
				physx::string::strcpy_s( begin(), capacity(), cStr );
			}
			else
			{
				resize( 0 );
			}
		}
		return *this;
	}

	void truncate( physx::PxU32 newLength )
	{
		if( newLength < length )
		{
			length = newLength;
			begin()[length] = '\0';
		}
	}

	void serialize( physx::PxFileBuf& stream ) const
	{
		stream << length;
		stream.write( begin(), length );
	}

	void deserialize( physx::PxFileBuf& stream )
	{
		physx::PxU32 len;
		stream >> len;
		if( len > 0 )
		{
			resize( len + 1 );
			stream.read( begin(), len );
			begin()[len] = '\0';
			length = len;
		}
		else
		{
			erase();
		}
	}

	physx::PxU32	len() const { return length; }

	/* PH: Cast operator not allowed by coding guidelines, and evil in general anyways
	operator const char* () const
	{
	return capacity() ? begin() : "";
	}
	*/
	const char* c_str() const
	{
		return capacity() > 0 ? begin() : "";
	}

	bool operator==(const ApexSimpleString& s) const
	{
		return strcmp(c_str(), s.c_str()) == 0;
	}
	bool operator==(const char* s) const
	{
		if ( s == NULL ) s = "";
		return strcmp(c_str(), s) == 0;
	}
	bool operator < (const ApexSimpleString& s) const
	{
		return strcmp(c_str(), s.c_str()) < 0;
	}

	ApexSimpleString& operator += ( const ApexSimpleString& s )
	{
		expandTo( length + s.length );
		physx::string::strcpy_s( begin() + length, capacity()-length, s.c_str() );
		length += s.length;
		return *this;
	}

	ApexSimpleString& operator += ( char c )
	{
		expandTo( length + 1 );
		begin()[length++] = c;
		begin()[length] = '\0';
		return *this;
	}

	ApexSimpleString operator + ( const ApexSimpleString& s )
	{
		ApexSimpleString sum = *this;
		sum += s;
		return sum;
	}

	ApexSimpleString &	clear()
	{
		if( capacity() )
		{
			begin()[0] = '\0';
		}
		length = 0;
		return *this;
	}

	ApexSimpleString &	erase()
	{
		resize( 0 );
		return clear();
	}

private:

	void expandTo( physx::PxU32 stringCapacity )
	{
		if( stringCapacity+1 > capacity() )
		{
			resize( 2*stringCapacity+1 );
		}
	}

	physx::PxU32 length;
};

PX_INLINE ApexSimpleString operator + ( const ApexSimpleString& s1, const ApexSimpleString& s2 )
{
	ApexSimpleString result = s1;
	result += s2;
	return result;
}

}} // end namespace physx::apex

#endif
