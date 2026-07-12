#ifndef _CLASSBUILDERINCLUDE_H
#define _CLASSBUILDERINCLUDE_H

// Date, Time & Version defines
#define DATAMODEL_DATE    20260625
#define DATAMODEL_TIME    194808
#define DATAMODEL_VERSION 3

// Context define declarations

// Forward extern class declarations
class CClassBuilderDoc;
class QDialog;
class ParseLogInterface;
class SourceLogInterface;
class CbArchive;
class CbPainter;

//@START_USER1
#include "CbSerialize.h"
//@END_USER1

// Defines needed for relations between templated classes

// Type declarations
//@START_DECLARATION_791 AccessType
typedef int AccessType;

#define PUBLIC    0
#define PROTECTED 1
#define PRIVATE   2
#define NONE      3
//@END_DECLARATION_791

//@START_DECLARATION_4538 CursorType
enum CursorType
{
    CURSOR_WE = 0,
    CURSOR_NS = 1,
    CURSOR_NWSE = 2,
    CURSOR_NESW = 3,
    CURSOR_4WAY = 4
};
//@END_DECLARATION_4538

//@START_DECLARATION_4772 CbColorRef
// CbColorRef (packed 0x00BBGGRR colour) + Cb_RGB / Cb_GetR/G/B helpers. One
// MFC-free definition shared by the generated model and the hand-written
// painter (CbPainter.h includes it too) -- see CbColor.h.
#include "CbColor.h"
//@END_DECLARATION_4772

//@START_DECLARATION_5853 VerbosityType
enum VerbosityType
{
    VERBOSITY_OFF      = 0,
    VERBOSITY_ARGUMENT = 1,
    VERBOSITY_STATIC   = 2
};
//@END_DECLARATION_5853

//@START_DECLARATION_11956 GoType
enum GoType {
    STOP = 0,
    LASTEQUAL = 1,
    FIRSTEQUAL = 2,
    IMPROVE = 3
};

//@END_DECLARATION_11956

//@START_DECLARATION_23440 Phase
enum Phase {
    None_Phase           = 0,
    Analysis_Phase       = 1,
    Design_Phase         = 2,
    Implementation_Phase = 3,
    Test_Phase           = 4,
    Complete_Phase       = 5
};
//@END_DECLARATION_23440

//@START_DECLARATION_34201 SeqType
enum SeqType {
    SEQ_NONE  = 0,
    SEQ_1     = 1,
    SEQ_1_1_1 = 2,
    SEQ_a     = 3,
    SEQ_a_a_a = 4,
    SEQ_A     = 5,
    SEQ_A_A_A = 6,
};

//@END_DECLARATION_34201

//@START_DECLARATION_39827 RefreshCallback
typedef void (*RefreshCallback)(void* context);
//@END_DECLARATION_39827

//@START_DECLARATION_40897 CbFontRole
enum CbFontRole {
    CBF_CLASS_NAME      = 0,
    CBF_ABSTRACT_CLASS  = 1,
    CBF_MEMBER          = 2,
    CBF_METHOD          = 3,
    CBF_STATIC_MEMBER   = 4,
    CBF_STATIC_METHOD   = 5,
    CBF_RELATION        = 6,
    CBF_LIFELINE        = 7,
    CBF_SIGNAL          = 8,
    CBF_LABEL           = 9,
};
//@END_DECLARATION_40897

// Forward class declarations
class Argument;
class BaseClass;
class Class;
class ClassGroup;
class Constructor;
class ConstructorIncludeMethod;
class DataModel;
class DataModelDoc;
class DataModelDocObject;
class Destructor;
class DestructorIncludeMethod;
class ExternClass;
class ExternClasses;
class FindMethod;
class FixedMethod;
class FromRelation;
class FromRelationMethod;
class GetMemberMethod;
class Group;
class Gti;
class Inherit;
class IsClassMethod;
class Member;
class MemberAndMethodGroup;
class MemberArgument;
class MemberMethod;
class Method;
class OtherType;
class OtherTypes;
class Relation;
class ReplaceConstructor;
class ReplaceConstructorIncludeMethod;
class SerializeConstructor;
class SerializeConstructorIncludeMethod;
class SerializeMethod;
class SerializeRelationsMethod;
class SetMemberMethod;
class ToRelation;
class Type;
class Variable;
class WrapMemberMethod;
class RelationMember;
class UniqueValueTree;
class FindUniqueValueTreeMethod;
class ValueTree;
class FindValueTreeMethod;
class FindReverseValueTreeMethod;
class MacroMethod;
class MacroMethods;
class FromRelationMacroMethods;
class ToRelationMacroMethods;
class MultiMacroMethods;
class MultiOwnedMacroMethods;
class SingleMacroMethods;
class SingleOwnedMacroMethods;
class StaticMultiMacroMethods;
class StaticMultiOwnedMacroMethods;
class ClassDiagram;
class ClassDiagramShape;
class ClassShape;
class RelationShape;
class InheritShape;
class MemberShape;
class MethodShape;
class Shape;
class NoteShape;
class ConnectionShape;
class ConnectionSegment;
class RelationAggregationStartSegment;
class RelationAssociationStartSegment;
class RelationMultiEndSegment;
class RelationSingleEndSegment;
class InheritStartSegment;
class InheritEndSegment;
class CleanupReferencesMethod;
class RemoveReferencesMethod;
class RestoreReferencesMethod;
class SaveReferencesMethod;
class NoteShapePoint;
class UndoBase;
class RedoBase;
class UndoNew;
class RedoNew;
class UndoDelete;
class RedoDelete;
class UndoSubDelete;
class UndoChange;
class RedoChange;
class UndoSubChange;
class CbStringBuilder;
class Grid;
class Column;
class Row;
class GridPoint;
class GridObject;
class GridRelation;
class ExceptionSpecification;
class ExceptionSpecificationType;
class RelationDiagramOnlyShape;
class DependencyShape;
class DependencyStartSegment;
class DependencyEndSegment;
class ContextDeclaration;
class Context;
class MemberContext;
class MemberAndMethodGroupContext;
class MethodContext;
class ClassContext;
class ClassGroupContext;
class MetaGroup;
class SequenceDiagram;
class SequenceDiagramShape;
class LifeLineShape;
class ClassLifeLineShape;
class ParentActivationShape;
class RootActivationShape;
class ChildActivationShape;
class SignalShape;
class Actors;
class Actor;
class ActorLifeLineShape;
class SDNoteShape;
class SDNoteShapePoint;
class UndoChangeDoc;
class RedoChangeDoc;
class AvlTree;
class FindAvlTreeMethod;
class FindReverseAvlTreeMethod;
class FindEqualOrSmallerAvlTreeMethod;
class FindEqualOrBiggerAvlTreeMethod;
class Property;
class PropertyInteger;
class PropertyReal;
class PropertyString;
class Contrib;
class SequenceDiagramViewModel;
class SequenceDiagramViewModelSelection;
class ClassDiagramViewModel;
class ClassDiagramViewModelSelection;
class TreeViewModel;
class CbViewLock;

// Needed ClassBuilder include files
#include "CB_SingleOwned.h"
#include "CB_MultiOwned.h"
#include "CB_AvlTreeOwned.h"
#include "CB_StaticMultiOwned.h"

// Make sure the inline implementations are skipped
#ifdef CB_INLINES
#undef CB_INLINES
#endif

// Include classes, for declarations
#include "DataModelDocObject.h"
#include "Gti.h"
#include "Variable.h"
#include "Argument.h"
#include "ExceptionSpecificationType.h"
#include "Type.h"
#include "Shape.h"
#include "ClassDiagramViewModelSelection.h"
#include "ClassDiagramShape.h"
#include "ConnectionSegment.h"
#include "ConnectionShape.h"
#include "InheritShape.h"
#include "Inherit.h"
#include "MethodShape.h"
#include "Context.h"
#include "MethodContext.h"
#include "SequenceDiagramViewModelSelection.h"
#include "SequenceDiagramShape.h"
#include "ParentActivationShape.h"
#include "SignalShape.h"
#include "ChildActivationShape.h"
#include "Method.h"
#include "MemberMethod.h"
#include "MemberArgument.h"
#include "GetMemberMethod.h"
#include "SetMemberMethod.h"
#include "RelationMember.h"
#include "MemberShape.h"
#include "MemberContext.h"
#include "Member.h"
#include "Group.h"
#include "MemberAndMethodGroupContext.h"
#include "MemberAndMethodGroup.h"
#include "ClassShape.h"
#include "LifeLineShape.h"
#include "ClassLifeLineShape.h"
#include "BaseClass.h"
#include "ExternClass.h"
#include "FromRelationMethod.h"
#include "FromRelation.h"
#include "ToRelation.h"
#include "RelationShape.h"
#include "Relation.h"
#include "FixedMethod.h"
#include "ConstructorIncludeMethod.h"
#include "IsClassMethod.h"
#include "ClassContext.h"
#include "Class.h"
#include "ClassGroupContext.h"
#include "ClassGroup.h"
#include "Constructor.h"
#include "ContextDeclaration.h"
#include "MetaGroup.h"
#include "DataModel.h"
#include "ExternClasses.h"
#include "OtherTypes.h"
#include "ClassDiagram.h"
#include "RedoBase.h"
#include "SequenceDiagram.h"
#include "ActorLifeLineShape.h"
#include "Actor.h"
#include "TreeViewModel.h"
#include "DataModelDoc.h"
#include "DestructorIncludeMethod.h"
#include "Destructor.h"
#include "FindMethod.h"
#include "OtherType.h"
#include "ReplaceConstructorIncludeMethod.h"
#include "ReplaceConstructor.h"
#include "SerializeConstructorIncludeMethod.h"
#include "SerializeConstructor.h"
#include "SerializeMethod.h"
#include "SerializeRelationsMethod.h"
#include "WrapMemberMethod.h"
#include "UniqueValueTree.h"
#include "FindUniqueValueTreeMethod.h"
#include "ValueTree.h"
#include "FindValueTreeMethod.h"
#include "FindReverseValueTreeMethod.h"
#include "MacroMethod.h"
#include "MacroMethods.h"
#include "FromRelationMacroMethods.h"
#include "ToRelationMacroMethods.h"
#include "MultiMacroMethods.h"
#include "MultiOwnedMacroMethods.h"
#include "SingleMacroMethods.h"
#include "SingleOwnedMacroMethods.h"
#include "StaticMultiMacroMethods.h"
#include "StaticMultiOwnedMacroMethods.h"
#include "NoteShapePoint.h"
#include "NoteShape.h"
#include "RelationAggregationStartSegment.h"
#include "RelationAssociationStartSegment.h"
#include "RelationMultiEndSegment.h"
#include "RelationSingleEndSegment.h"
#include "InheritStartSegment.h"
#include "InheritEndSegment.h"
#include "CleanupReferencesMethod.h"
#include "RemoveReferencesMethod.h"
#include "RestoreReferencesMethod.h"
#include "SaveReferencesMethod.h"
#include "UndoBase.h"
#include "UndoNew.h"
#include "RedoNew.h"
#include "UndoDelete.h"
#include "RedoDelete.h"
#include "UndoSubDelete.h"
#include "UndoChange.h"
#include "RedoChange.h"
#include "UndoSubChange.h"
#include "CbStringBuilder.h"
#include "GridPoint.h"
#include "Column.h"
#include "Row.h"
#include "GridRelation.h"
#include "GridObject.h"
#include "Grid.h"
#include "ExceptionSpecification.h"
#include "RelationDiagramOnlyShape.h"
#include "DependencyShape.h"
#include "DependencyStartSegment.h"
#include "DependencyEndSegment.h"
#include "RootActivationShape.h"
#include "Actors.h"
#include "SDNoteShapePoint.h"
#include "SDNoteShape.h"
#include "UndoChangeDoc.h"
#include "RedoChangeDoc.h"
#include "AvlTree.h"
#include "FindAvlTreeMethod.h"
#include "FindReverseAvlTreeMethod.h"
#include "FindEqualOrSmallerAvlTreeMethod.h"
#include "FindEqualOrBiggerAvlTreeMethod.h"
#include "Property.h"
#include "PropertyInteger.h"
#include "PropertyReal.h"
#include "PropertyString.h"
#include "Contrib.h"
#include "SequenceDiagramViewModel.h"
#include "ClassDiagramViewModel.h"
#include "CbViewLock.h"


// Include classes again, for inline implementation
#define CB_INLINES
#include "BaseClass.h"
#include "Class.h"
#include "DataModel.h"
#include "DataModelDoc.h"
#include "Gti.h"
#include "Member.h"
#include "Method.h"
#include "ClassDiagram.h"
#include "ClassShape.h"
#include "RelationShape.h"
#include "Shape.h"
#include "NoteShape.h"
#include "UndoBase.h"
#include "RedoBase.h"
#include "UndoChange.h"
#include "RedoChange.h"
#include "UndoSubChange.h"
#include "CbStringBuilder.h"
#include "Grid.h"
#include "Column.h"
#include "Row.h"
#include "GridObject.h"
#include "GridRelation.h"
#include "ExceptionSpecificationType.h"
#include "RelationDiagramOnlyShape.h"
#include "DependencyShape.h"
#include "ContextDeclaration.h"
#include "SequenceDiagram.h"
#include "LifeLineShape.h"
#include "ClassLifeLineShape.h"
#include "ChildActivationShape.h"
#include "SignalShape.h"
#include "Actor.h"
#include "SDNoteShape.h"
#include "UndoChangeDoc.h"
#include "Property.h"
#include "PropertyInteger.h"
#include "PropertyReal.h"
#include "PropertyString.h"
#include "Contrib.h"
#include "TreeViewModel.h"

//@START_USER2
#define NL "\015\012"

#define MODIFICATION            1
#define MOD_ADD                 MODIFICATION<<0
#define MOD_UPDATE              MODIFICATION<<1
#define MOD_REMOVE              MODIFICATION<<2
#define MOD_SORT                MODIFICATION<<3
#define MOD_REFRESH             MODIFICATION<<4

#define ICON_FILE                           0
#define ICON_FILESELECTED                   1

#define ICON_CLASS                          2
#define ICON_INHERIT                        3

#define ICON_PUBLIC_MEMBER                  4
#define ICON_PROTECTED_MEMBER               5
#define ICON_PRIVATE_MEMBER                 6

#define ICON_PUBLIC_CONSTRUCTOR             7
#define ICON_PROTECTED_CONSTRUCTOR          8
#define ICON_PRIVATE_CONSTRUCTOR            9

#define ICON_PUBLIC_DESTRUCTOR              10
#define ICON_PROTECTED_DESTRUCTOR           11
#define ICON_PRIVATE_DESTRUCTOR             12

#define ICON_PUBLIC_METHOD                  13
#define ICON_PROTECTED_METHOD               14
#define ICON_PRIVATE_METHOD                 15

#define ICON_SINGLE_ACT                     16
#define ICON_SINGLE_PAS                     17
#define ICON_OWNED_SINGLE_PAS               18
#define ICON_OWNED_SINGLE_ACT               19

#define ICON_MULTI_ACT                      20
#define ICON_MULTI_PAS                      21
#define ICON_OWNED_MULTI_ACT                22
#define ICON_OWNED_MULTI_PAS                23

#define ICON_STATIC_MULTI_ACT               24
#define ICON_STATIC_MULTI_PAS               25
#define ICON_STATIC_OWNED_MULTI_ACT         26
#define ICON_STATIC_OWNED_MULTI_PAS         27

#define ICON_CR_SINGLE_ACT                  28
#define ICON_CR_SINGLE_PAS                  29
#define ICON_CR_OWNED_SINGLE_ACT            30
#define ICON_CR_OWNED_SINGLE_PAS            31

#define ICON_CR_MULTI_ACT                   32
#define ICON_CR_MULTI_PAS                   33
#define ICON_CR_OWNED_MULTI_ACT             34
#define ICON_CR_OWNED_MULTI_PAS             35

#define ICON_CR_STATIC_MULTI_ACT            36
#define ICON_CR_STATIC_MULTI_PAS            37
#define ICON_CR_STATIC_OWNED_MULTI_ACT      38
#define ICON_CR_STATIC_OWNED_MULTI_PAS      39

#define ICON_TYPE                           40
#define ICON_ARGUMENT                       41
#define ICON_METHODGROUP                    42
#define ICON_MEMBERGROUP                    43
#define ICON_MEMBERMETHODGROUP              44

#define ICON_CLASSDIAGRAM                   45

#define INLINE_OFFSET                       39
#define ICON_PUBLIC_INLINE_CONSTRUCTOR      46
#define ICON_PROTECTED_INLINE_CONSTRUCTOR   47
#define ICON_PRIVATE_INLINE_CONSTRUCTOR     48

#define ICON_PUBLIC_INLINE_DESTRUCTOR       49
#define ICON_PROTECTED_INLINE_DESTRUCTOR    50
#define ICON_PRIVATE_INLINE_DESTRUCTOR      51

#define ICON_PUBLIC_INLINE_METHOD           52
#define ICON_PROTECTED_INLINE_METHOD        53
#define ICON_PRIVATE_INLINE_METHOD          54

#define ICON_EXTERNCLASS                    55

#define EMPTY_OFFSET                        49
#define ICON_PUBLIC_EMPTY_CONSTRUCTOR       56
#define ICON_PROTECTED_EMPTY_CONSTRUCTOR    57
#define ICON_PRIVATE_EMPTY_CONSTRUCTOR      58

#define ICON_PUBLIC_EMPTY_DESTRUCTOR        59
#define ICON_PROTECTED_EMPTY_DESTRUCTOR     60
#define ICON_PRIVATE_EMPTY_DESTRUCTOR       61

#define ICON_PUBLIC_EMPTY_METHOD            62
#define ICON_PROTECTED_EMPTY_METHOD         63
#define ICON_PRIVATE_EMPTY_METHOD           64

#define ICON_SEQUENCEDIAGRAM                65
#define ICON_ACTOR                          66

// Untouched INLINE method: hollow white core like the empty icons, but the
// rim in the darker inline shade -- body place (inline) picks the rim
// colour, the untouched state picks the hollow core (JV 2026-07-12).
#define ICON_PUBLIC_EMPTY_INLINE_METHOD     67
#define ICON_PROTECTED_EMPTY_INLINE_METHOD  68
#define ICON_PRIVATE_EMPTY_INLINE_METHOD    69

inline CbString WrapArguments(CbString line, int maxLen = 80)
{
    if (line.GetLength() <= maxLen)
        return line;

    CbString indent;
    for (int i = 0; i < line.GetLength(); i++)
    {
        indent += ' ';
        if (line[i] == '(')
            break;
    }

    CbString result;
    while (line.GetLength() > maxLen)
    {
        int lastSplit = 0;
        for (int i = indent.GetLength(); i < maxLen; i++)
        {
            if (line[i] == ',' && line[i+1] == ' ')
                lastSplit = i;
        }

        if (!lastSplit)
        {
            for (int i = maxLen; i < line.GetLength(); i++)
            {
                if (line[i] == ',' && line[i+1] == ' ')
                {
                    lastSplit = i;
                    break;
                }
            }
        }

        if (lastSplit)
        {
            result += line.Left(lastSplit+1) + NL;
            line = indent + line.Mid(lastSplit+2);
        }
        else
        {
            break;
        }
    }

    result += line;
    return result;
}

inline CbRect& operator *=(CbRect& rect, const CbRect& otherRect)
{
    // Allow an initial point rectangle, do not take that into acount
    if (rect.BottomRight() == rect.TopLeft())
    {
        rect = otherRect;
    }
    else
    {
        if (rect.top > otherRect.top)
        {
            rect.top = otherRect.top;
        }
        if (rect.bottom < otherRect.bottom)
        {
            rect.bottom = otherRect.bottom;
        }
        if (rect.left > otherRect.left)
        {
            rect.left = otherRect.left;
        }
        if (rect.right < otherRect.right)
        {
            rect.right = otherRect.right;
        }
    }

    return rect;
}

// (The MFC dialog helpers -- GETBUTTON/GETCOMBOBOX/... macros and the
//  CComboBox SelectString -- were removed with the MFC shell; the Qt dialogs
//  have their own helpers in qt/QtComboHelpers.h.)
//@END_USER2

#endif
