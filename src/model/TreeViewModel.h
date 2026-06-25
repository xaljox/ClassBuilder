/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          TreeViewModel.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'TreeViewModel'
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
#ifndef _TREEVIEWMODEL_H
#define _TREEVIEWMODEL_H

//@START_USER1
//@END_USER1



class TreeViewModel
{
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)

//@START_USER2
//@END_USER2

// Members
private:
    RefreshCallback _closeFn;
    void* _refreshCtx;
    RefreshCallback _refreshFn;
    bool _showPublicMembers;
    bool _showProtectedMembers;
    bool _showPrivateMembers;
    bool _showPublicMethods;
    bool _showProtectedMethods;
    bool _showPrivateMethods;
    bool _showAnalysisPhase;
    bool _showDesignPhase;
    bool _showImplementationPhase;
    bool _showTestPhase;
    bool _showCompletePhase;
    Gti* _subTree;
    bool _showOnlyClassesWithoutConstructor;
    bool _showStaticMembers;
    bool _showNonStaticMembers;
    bool _showStaticMethods;
    bool _showNonStaticMethods;
    bool _showInheritance;
    bool _showMultiRelations;
    bool _showSingleRelations;
    bool _showAggregationRelations;
    bool _showNonAggregationRelations;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();

protected:

public:
    TreeViewModel(DataModelDoc* pDataModelDoc, RefreshCallback refreshFn,
                  RefreshCallback closeFn, void* refreshCtx, Gti* subTree = 0);
    virtual ~TreeViewModel();
    void* GetRefreshCtx() const;
    void Refresh() const;
    bool GetShowAggregationRelations() const;
    void SetShowAggregationRelations(bool showAggregationRelations);
    bool GetShowAnalysisPhase() const;
    void SetShowAnalysisPhase(bool showAnalysisPhase);
    bool GetShowCompletePhase() const;
    void SetShowCompletePhase(bool showCompletePhase);
    bool GetShowDesignPhase() const;
    void SetShowDesignPhase(bool showDesignPhase);
    bool GetShowImplementationPhase() const;
    void SetShowImplementationPhase(bool showImplementationPhase);
    bool GetShowInheritance() const;
    void SetShowInheritance(bool showInheritance);
    bool GetShowMultiRelations() const;
    void SetShowMultiRelations(bool showMultiRelations);
    bool GetShowNonAggregationRelations() const;
    void SetShowNonAggregationRelations(bool showNonAggregationRelations);
    bool GetShowNonStaticMembers() const;
    void SetShowNonStaticMembers(bool showNonStaticMembers);
    bool GetShowNonStaticMethods() const;
    void SetShowNonStaticMethods(bool showNonStaticMethods);
    bool GetShowOnlyClassesWithoutConstructor() const;
    void SetShowOnlyClassesWithoutConstructor(bool showOnlyClassesWithoutConstructor);
    bool GetShowPrivateMembers() const;
    void SetShowPrivateMembers(bool showPrivateMembers);
    bool GetShowPrivateMethods() const;
    void SetShowPrivateMethods(bool showPrivateMethods);
    bool GetShowProtectedMembers() const;
    void SetShowProtectedMembers(bool showProtectedMembers);
    bool GetShowProtectedMethods() const;
    void SetShowProtectedMethods(bool showProtectedMethods);
    bool GetShowPublicMembers() const;
    void SetShowPublicMembers(bool showPublicMembers);
    bool GetShowPublicMethods() const;
    void SetShowPublicMethods(bool showPublicMethods);
    bool GetShowSingleRelations() const;
    void SetShowSingleRelations(bool showSingleRelations);
    bool GetShowStaticMembers() const;
    void SetShowStaticMembers(bool showStaticMembers);
    bool GetShowStaticMethods() const;
    void SetShowStaticMethods(bool showStaticMethods);
    bool GetShowTestPhase() const;
    void SetShowTestPhase(bool showTestPhase);
    Gti* GetSubTree() const;
};

#endif


#ifdef CB_INLINES
#ifndef _TREEVIEWMODEL_H_INLINES
#define _TREEVIEWMODEL_H_INLINES

/*@NOTE_40828
Returns the value of member '_showAggregationRelations'.
*/
inline bool TreeViewModel::GetShowAggregationRelations() const
{//@CODE_40828
    return _showAggregationRelations;
}//@CODE_40828



/*@NOTE_40829
Set the value of member '_showAggregationRelations' to 'showAggregationRelations'.
*/
inline void TreeViewModel::SetShowAggregationRelations(bool showAggregationRelations)
{//@CODE_40829
    _showAggregationRelations = showAggregationRelations;
}//@CODE_40829



/*@NOTE_40752
Returns the value of member '_showAnalysisPhase'.
*/
inline bool TreeViewModel::GetShowAnalysisPhase() const
{//@CODE_40752
    return _showAnalysisPhase;
}//@CODE_40752



/*@NOTE_40753
Set the value of member '_showAnalysisPhase' to 'showAnalysisPhase'.
*/
inline void TreeViewModel::SetShowAnalysisPhase(bool showAnalysisPhase)
{//@CODE_40753
    _showAnalysisPhase = showAnalysisPhase;
}//@CODE_40753



/*@NOTE_40768
Returns the value of member '_showCompletePhase'.
*/
inline bool TreeViewModel::GetShowCompletePhase() const
{//@CODE_40768
    return _showCompletePhase;
}//@CODE_40768



/*@NOTE_40769
Set the value of member '_showCompletePhase' to 'showCompletePhase'.
*/
inline void TreeViewModel::SetShowCompletePhase(bool showCompletePhase)
{//@CODE_40769
    _showCompletePhase = showCompletePhase;
}//@CODE_40769



/*@NOTE_40756
Returns the value of member '_showDesignPhase'.
*/
inline bool TreeViewModel::GetShowDesignPhase() const
{//@CODE_40756
    return _showDesignPhase;
}//@CODE_40756



/*@NOTE_40757
Set the value of member '_showDesignPhase' to 'showDesignPhase'.
*/
inline void TreeViewModel::SetShowDesignPhase(bool showDesignPhase)
{//@CODE_40757
    _showDesignPhase = showDesignPhase;
}//@CODE_40757



/*@NOTE_40760
Returns the value of member '_showImplementationPhase'.
*/
inline bool TreeViewModel::GetShowImplementationPhase() const
{//@CODE_40760
    return _showImplementationPhase;
}//@CODE_40760



/*@NOTE_40761
Set the value of member '_showImplementationPhase' to 'showImplementationPhase'.
*/
inline void TreeViewModel::SetShowImplementationPhase(bool showImplementationPhase)
{//@CODE_40761
    _showImplementationPhase = showImplementationPhase;
}//@CODE_40761



/*@NOTE_40816
Returns the value of member '_showInheritance'.
*/
inline bool TreeViewModel::GetShowInheritance() const
{//@CODE_40816
    return _showInheritance;
}//@CODE_40816



/*@NOTE_40817
Set the value of member '_showInheritance' to 'showInheritance'.
*/
inline void TreeViewModel::SetShowInheritance(bool showInheritance)
{//@CODE_40817
    _showInheritance = showInheritance;
}//@CODE_40817



/*@NOTE_40820
Returns the value of member '_showMultiRelations'.
*/
inline bool TreeViewModel::GetShowMultiRelations() const
{//@CODE_40820
    return _showMultiRelations;
}//@CODE_40820



/*@NOTE_40821
Set the value of member '_showMultiRelations' to 'showMultiRelations'.
*/
inline void TreeViewModel::SetShowMultiRelations(bool showMultiRelations)
{//@CODE_40821
    _showMultiRelations = showMultiRelations;
}//@CODE_40821



/*@NOTE_40832
Returns the value of member '_showNonAggregationRelations'.
*/
inline bool TreeViewModel::GetShowNonAggregationRelations() const
{//@CODE_40832
    return _showNonAggregationRelations;
}//@CODE_40832



/*@NOTE_40833
Set the value of member '_showNonAggregationRelations' to 'showNonAggregationRelations'.
*/
inline void TreeViewModel::SetShowNonAggregationRelations(bool showNonAggregationRelations)
{//@CODE_40833
    _showNonAggregationRelations = showNonAggregationRelations;
}//@CODE_40833



/*@NOTE_40804
Returns the value of member '_showNonStaticMembers'.
*/
inline bool TreeViewModel::GetShowNonStaticMembers() const
{//@CODE_40804
    return _showNonStaticMembers;
}//@CODE_40804



/*@NOTE_40805
Set the value of member '_showNonStaticMembers' to 'showNonStaticMembers'.
*/
inline void TreeViewModel::SetShowNonStaticMembers(bool showNonStaticMembers)
{//@CODE_40805
    _showNonStaticMembers = showNonStaticMembers;
}//@CODE_40805



/*@NOTE_40812
Returns the value of member '_showNonStaticMethods'.
*/
inline bool TreeViewModel::GetShowNonStaticMethods() const
{//@CODE_40812
    return _showNonStaticMethods;
}//@CODE_40812



/*@NOTE_40813
Set the value of member '_showNonStaticMethods' to 'showNonStaticMethods'.
*/
inline void TreeViewModel::SetShowNonStaticMethods(bool showNonStaticMethods)
{//@CODE_40813
    _showNonStaticMethods = showNonStaticMethods;
}//@CODE_40813



/*@NOTE_40794
Returns the value of member '_showOnlyClassesWithoutConstructor'.
*/
inline bool TreeViewModel::GetShowOnlyClassesWithoutConstructor() const
{//@CODE_40794
    return _showOnlyClassesWithoutConstructor;
}//@CODE_40794



/*@NOTE_40795
Set the value of member '_showOnlyClassesWithoutConstructor' to 'showOnlyClassesWithoutConstructor'.
*/
inline void TreeViewModel::SetShowOnlyClassesWithoutConstructor(bool showOnlyClassesWithoutConstructor)
{//@CODE_40795
    _showOnlyClassesWithoutConstructor = showOnlyClassesWithoutConstructor;
}//@CODE_40795



/*@NOTE_40736
Returns the value of member '_showPrivateMembers'.
*/
inline bool TreeViewModel::GetShowPrivateMembers() const
{//@CODE_40736
    return _showPrivateMembers;
}//@CODE_40736



/*@NOTE_40737
Set the value of member '_showPrivateMembers' to 'showPrivateMembers'.
*/
inline void TreeViewModel::SetShowPrivateMembers(bool showPrivateMembers)
{//@CODE_40737
    _showPrivateMembers = showPrivateMembers;
}//@CODE_40737



/*@NOTE_40748
Returns the value of member '_showPrivateMethods'.
*/
inline bool TreeViewModel::GetShowPrivateMethods() const
{//@CODE_40748
    return _showPrivateMethods;
}//@CODE_40748



/*@NOTE_40749
Set the value of member '_showPrivateMethods' to 'showPrivateMethods'.
*/
inline void TreeViewModel::SetShowPrivateMethods(bool showPrivateMethods)
{//@CODE_40749
    _showPrivateMethods = showPrivateMethods;
}//@CODE_40749



/*@NOTE_40732
Returns the value of member '_showProtectedMembers'.
*/
inline bool TreeViewModel::GetShowProtectedMembers() const
{//@CODE_40732
    return _showProtectedMembers;
}//@CODE_40732



/*@NOTE_40733
Set the value of member '_showProtectedMembers' to 'showProtectedMembers'.
*/
inline void TreeViewModel::SetShowProtectedMembers(bool showProtectedMembers)
{//@CODE_40733
    _showProtectedMembers = showProtectedMembers;
}//@CODE_40733



/*@NOTE_40744
Returns the value of member '_showProtectedMethods'.
*/
inline bool TreeViewModel::GetShowProtectedMethods() const
{//@CODE_40744
    return _showProtectedMethods;
}//@CODE_40744



/*@NOTE_40745
Set the value of member '_showProtectedMethods' to 'showProtectedMethods'.
*/
inline void TreeViewModel::SetShowProtectedMethods(bool showProtectedMethods)
{//@CODE_40745
    _showProtectedMethods = showProtectedMethods;
}//@CODE_40745



/*@NOTE_40728
Returns the value of member '_showPublicMembers'.
*/
inline bool TreeViewModel::GetShowPublicMembers() const
{//@CODE_40728
    return _showPublicMembers;
}//@CODE_40728



/*@NOTE_40729
Set the value of member '_showPublicMembers' to 'showPublicMembers'.
*/
inline void TreeViewModel::SetShowPublicMembers(bool showPublicMembers)
{//@CODE_40729
    _showPublicMembers = showPublicMembers;
}//@CODE_40729



/*@NOTE_40740
Returns the value of member '_showPublicMethods'.
*/
inline bool TreeViewModel::GetShowPublicMethods() const
{//@CODE_40740
    return _showPublicMethods;
}//@CODE_40740



/*@NOTE_40741
Set the value of member '_showPublicMethods' to 'showPublicMethods'.
*/
inline void TreeViewModel::SetShowPublicMethods(bool showPublicMethods)
{//@CODE_40741
    _showPublicMethods = showPublicMethods;
}//@CODE_40741



/*@NOTE_40824
Returns the value of member '_showSingleRelations'.
*/
inline bool TreeViewModel::GetShowSingleRelations() const
{//@CODE_40824
    return _showSingleRelations;
}//@CODE_40824



/*@NOTE_40825
Set the value of member '_showSingleRelations' to 'showSingleRelations'.
*/
inline void TreeViewModel::SetShowSingleRelations(bool showSingleRelations)
{//@CODE_40825
    _showSingleRelations = showSingleRelations;
}//@CODE_40825



/*@NOTE_40800
Returns the value of member '_showStaticMembers'.
*/
inline bool TreeViewModel::GetShowStaticMembers() const
{//@CODE_40800
    return _showStaticMembers;
}//@CODE_40800



/*@NOTE_40801
Set the value of member '_showStaticMembers' to 'showStaticMembers'.
*/
inline void TreeViewModel::SetShowStaticMembers(bool showStaticMembers)
{//@CODE_40801
    _showStaticMembers = showStaticMembers;
}//@CODE_40801



/*@NOTE_40808
Returns the value of member '_showStaticMethods'.
*/
inline bool TreeViewModel::GetShowStaticMethods() const
{//@CODE_40808
    return _showStaticMethods;
}//@CODE_40808



/*@NOTE_40809
Set the value of member '_showStaticMethods' to 'showStaticMethods'.
*/
inline void TreeViewModel::SetShowStaticMethods(bool showStaticMethods)
{//@CODE_40809
    _showStaticMethods = showStaticMethods;
}//@CODE_40809



/*@NOTE_40764
Returns the value of member '_showTestPhase'.
*/
inline bool TreeViewModel::GetShowTestPhase() const
{//@CODE_40764
    return _showTestPhase;
}//@CODE_40764



/*@NOTE_40765
Set the value of member '_showTestPhase' to 'showTestPhase'.
*/
inline void TreeViewModel::SetShowTestPhase(bool showTestPhase)
{//@CODE_40765
    _showTestPhase = showTestPhase;
}//@CODE_40765



/*@NOTE_40772
Returns the value of member '_subTree'.
*/
inline Gti* TreeViewModel::GetSubTree() const
{//@CODE_40772
    return _subTree;
}//@CODE_40772



//@START_USER3
//@END_USER3

#endif
#endif
