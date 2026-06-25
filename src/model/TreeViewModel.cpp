/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          TreeViewModel.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'TreeViewModel'
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
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_40722
Constructor method.
*/
TreeViewModel::TreeViewModel(DataModelDoc* pDataModelDoc,
                             RefreshCallback refreshFn, RefreshCallback closeFn,
                             void* refreshCtx, Gti* subTree) //@INIT_40722
    : _closeFn(closeFn)
    , _refreshCtx(refreshCtx)
    , _refreshFn(refreshFn)
    , _showPublicMembers(true)
    , _showProtectedMembers(true)
    , _showPrivateMembers(true)
    , _showPublicMethods(true)
    , _showProtectedMethods(true)
    , _showPrivateMethods(true)
    , _showAnalysisPhase(true)
    , _showDesignPhase(true)
    , _showImplementationPhase(true)
    , _showTestPhase(true)
    , _showCompletePhase(true)
    , _subTree(subTree)
    , _showOnlyClassesWithoutConstructor(false)
    , _showStaticMembers(true)
    , _showNonStaticMembers(true)
    , _showStaticMethods(true)
    , _showNonStaticMethods(true)
    , _showInheritance(true)
    , _showMultiRelations(true)
    , _showSingleRelations(true)
    , _showAggregationRelations(true)
    , _showNonAggregationRelations(true)
{//@CODE_40722
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_40722


/*@NOTE_40616
Destructor method.
*/
TreeViewModel::~TreeViewModel()
{//@CODE_40616
    DestructorInclude();

    // Put in your own code
    if (_closeFn) _closeFn(_refreshCtx);
}//@CODE_40616


void* TreeViewModel::GetRefreshCtx() const
{//@CODE_40784
    return _refreshCtx;
}//@CODE_40784


void TreeViewModel::Refresh() const
{//@CODE_40721
    if (_refreshFn)
    {
        _refreshFn(_refreshCtx); 
    }
}//@CODE_40721


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_40617
Method which must be called first in a constructor.
*/
void TreeViewModel::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
}


/*@NOTE_40618
Method which must be called first in a destructor.
*/
void TreeViewModel::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
