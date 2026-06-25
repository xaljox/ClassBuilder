/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Contrib.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Contrib'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
* Philips Digital Video Systems, Eindhoven, The Netherlands.
* Distributed under the GNU General Public License (GPL)
*
\******************************************************************************/
#ifndef _CONTRIB_H
#define _CONTRIB_H

//@START_USER1
//@END_USER1



class Contrib
{
    RELATION_NOFILTER_STATIC_MULTI_OWNED_ACTIVE(Contrib, Contrib, Contrib, Contrib)
    RELATION_STATIC_MULTI_OWNED_PASSIVE(Contrib, Contrib, Contrib, Contrib)

//@START_USER2
//@END_USER2

// Members
private:
    LifeLineShape* _pLeft;
    LifeLineShape* _pRight;
    int _needed;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    Contrib(LifeLineShape* pLLS1, LifeLineShape* pLLS2, int needed);
    ~Contrib();
    static Contrib* FindContrib(LifeLineShape* pLLS1, LifeLineShape* pLLS2);
    static Contrib* FindNextRightContrib(LifeLineShape* pRight,
                                         Contrib* startAfterContrib);
    static Contrib* FindRightContrib(LifeLineShape* pRight);
    void IncrementNeeded();
    int GetNeeded() const;
    void SetNeeded(int needed);
    LifeLineShape* GetLeft() const;
    LifeLineShape* GetRight() const;
};

#endif


#ifdef CB_INLINES
#ifndef _CONTRIB_H_INLINES
#define _CONTRIB_H_INLINES

inline void Contrib::IncrementNeeded()
{//@CODE_38569
    _needed++;
}//@CODE_38569



/*@NOTE_38551
Returns the value of member '_needed'.
*/
inline int Contrib::GetNeeded() const
{//@CODE_38551
    return _needed;
}//@CODE_38551



/*@NOTE_38559
Set the value of member '_needed' to 'needed'.
*/
inline void Contrib::SetNeeded(int needed)
{//@CODE_38559
    _needed = needed;
}//@CODE_38559



/*@NOTE_38547
Returns the value of member '_pLeft'.
*/
inline LifeLineShape* Contrib::GetLeft() const
{//@CODE_38547
    return _pLeft;
}//@CODE_38547



/*@NOTE_38549
Returns the value of member '_pRight'.
*/
inline LifeLineShape* Contrib::GetRight() const
{//@CODE_38549
    return _pRight;
}//@CODE_38549



//@START_USER3
//@END_USER3

#endif
#endif
