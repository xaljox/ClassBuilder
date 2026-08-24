/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          CbStringBuilder.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'CbStringBuilder'
*
* Modifications: @INSERT_MODIFICATIONS(* )
* August 24, 2026 14:03 JV License text updated
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*     Update comment header
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CBSTRINGBUILDER_H
#define _CBSTRINGBUILDER_H

//@START_USER1
//@END_USER1


/*@NOTE_7368
This is done to get more performance when handling large strings.
*/

class CbStringBuilder
    : public CbString
{

//@START_USER2
//@END_USER2

// Members
private:
    int _grow;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    CbStringBuilder(int size = 4096, int grow = 1024);
    ~CbStringBuilder();
    void Empty();
    const CbStringBuilder& operator +=(const char* lpsz);
    const CbStringBuilder& operator +=(char ch);
    const CbStringBuilder& operator +=(const CbString& string);
    const CbStringBuilder& operator =(const CbString& string);
};

#endif


#ifdef CB_INLINES
#ifndef _CBSTRINGBUILDER_H_INLINES
#define _CBSTRINGBUILDER_H_INLINES

/*@NOTE_7390
Construct it and give it already a buffer of size 'size'.
*/
inline CbStringBuilder::CbStringBuilder(int size, int grow) //@INIT_7390
    : CbString()
    , _grow(grow)
{//@CODE_7390
    (void)GetBuffer(size);
}//@CODE_7390



/*@NOTE_7370
Destructor method.
*/
inline CbStringBuilder::~CbStringBuilder()
{//@CODE_7370
    // Put in your own code
}//@CODE_7370



/*@NOTE_7394
Set length to zero, but don't free allocated buffer.
*/
inline void CbStringBuilder::Empty()
{//@CODE_7394
    (void)GetBufferSetLength(0);
}//@CODE_7394



inline const CbStringBuilder& CbStringBuilder::operator +=(const char* lpsz)
{//@CODE_7395
    int addLen = lpsz ? int(strlen(lpsz)) : 0;
    int newLen = GetLength() + addLen;
    Preallocate(((newLen / _grow) + 1) * _grow);
    CbString::operator+=(lpsz);
    return *this;
}//@CODE_7395



inline const CbStringBuilder& CbStringBuilder::operator +=(char ch)
{//@CODE_7396
    int newLen = GetLength() + 1;
    Preallocate(((newLen / _grow) + 1) * _grow);
    CbString::operator+=(ch);
    return *this;
}//@CODE_7396



inline const CbStringBuilder& CbStringBuilder::operator +=(const CbString& string)
{//@CODE_7397
    int newLen = GetLength() + string.GetLength();
    Preallocate(((newLen / _grow) + 1) * _grow);
    CbString::operator+=(string);
    return *this;
}//@CODE_7397



inline const CbStringBuilder& CbStringBuilder::operator =(const CbString& string)
{//@CODE_19520
    int newLen = string.GetLength();
    Preallocate(((newLen / _grow) + 1) * _grow);
    CbString::operator=(string);
    return *this;
}//@CODE_19520



/*@NOTE_7369
Method which must be called first in a constructor.
*/
inline void CbStringBuilder::ConstructorInclude()
{
}



/*@NOTE_7371
Method which must be called first in a destructor.
*/
inline void CbStringBuilder::DestructorInclude()
{
}



//@START_USER3
//@END_USER3

#endif
#endif
