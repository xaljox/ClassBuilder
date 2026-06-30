#include "StdAfx.h"
#include "CbSerialize.h"

// ===========================================================================
// CbClassRegistration — static member definitions
// ===========================================================================

CbClassRegistration* CbClassRegistration::_firstCbClassRegistration = (CbClassRegistration*)0;
CbClassRegistration* CbClassRegistration::_lastCbClassRegistration  = (CbClassRegistration*)0;
int                  CbClassRegistration::_countCbClassRegistration  = 0;

// Registers the class by name and factory pointer in the global linked list,
// so CbArchive can instantiate objects by name during deserialization.
CbClassRegistration::CbClassRegistration(const char* className,
                                         CreateObjectFuncPtr createObjectFuncPtr)
    : _createObjectFuncPtr(createObjectFuncPtr)
    , _className(0)
    , _index(0)
{
    // Append this instance to the global registration list
    _countCbClassRegistration++;
    if (_lastCbClassRegistration)
    {
        _nextCbClassRegistration = (CbClassRegistration*)0;
        _prevCbClassRegistration = _lastCbClassRegistration;
        _prevCbClassRegistration->_nextCbClassRegistration = this;
        _lastCbClassRegistration = this;
    }
    else
    {
        _prevCbClassRegistration = (CbClassRegistration*)0;
        _nextCbClassRegistration = (CbClassRegistration*)0;
        _firstCbClassRegistration = _lastCbClassRegistration = this;
    }

    if (className)
    {
        _className = new char[strlen(className) + 1];
        strcpy(_className, className);
    }
}

// Unlinks this registration from the global list and frees the class name copy.
CbClassRegistration::~CbClassRegistration()
{
    // Remove this instance from the global registration list
    if (_prevCbClassRegistration || _nextCbClassRegistration ||
        _firstCbClassRegistration == this)
    {
        _countCbClassRegistration--;

        if (_nextCbClassRegistration)
            _nextCbClassRegistration->_prevCbClassRegistration = _prevCbClassRegistration;
        else
            _lastCbClassRegistration = _prevCbClassRegistration;

        if (_prevCbClassRegistration)
            _prevCbClassRegistration->_nextCbClassRegistration = _nextCbClassRegistration;
        else
            _firstCbClassRegistration = _nextCbClassRegistration;

        _prevCbClassRegistration = (CbClassRegistration*)0;
        _nextCbClassRegistration = (CbClassRegistration*)0;
    }

    delete[] _className;
}

// Returns the registration entry whose class name matches, or null if not found.
CbClassRegistration* CbClassRegistration::FindCbClassRegistration(const char* className)
{
    for (CbClassRegistration* p = _firstCbClassRegistration; p;
         p = p->_nextCbClassRegistration)
    {
        if (strcmp(className, p->GetRegisteredClassName()) == 0)
            return p;
    }
    return 0;
}

// ===========================================================================
// CbArchive
// ===========================================================================

// Load archive backed by an existing input stream (no file ownership).
CbArchive::CbArchive(istream& is)
    : _istream(&is)
    , _ostream(0)
    , _loading(true)
    , _indexStore(0xFFFF)
    , _indexLoad(0xFFFF)
    , _registration(0)
    , _totalLength(0)
    , _objectsLoaded(0)
{
}

// Store archive backed by an existing output stream (no file ownership).
CbArchive::CbArchive(ostream& os)
    : _istream(0)
    , _ostream(&os)
    , _loading(false)
    , _indexStore(0xFFFF)
    , _indexLoad(0xFFFF)
    , _registration(0)
    , _totalLength(0)
    , _objectsLoaded(0)
{
}

// Flushes any pending stream output and frees the type-index table.
CbArchive::~CbArchive()
{
    delete[] _registration;

    if (_ostream)
    {
        _ostream->flush();
    }
}

// Reads the next object type from the archive. The first occurrence of a class carries
// its name as a length-prefixed string (high bit set in the index word); subsequent
// occurrences carry only the previously assigned short index, saving space.
CbClassRegistration* CbArchive::LoadObjectType()
{
    if (_indexLoad == 0xFFFF)
    {
        int registrationCnt = CbClassRegistration::GetCbClassRegistrationCount();
        _registration = new CbClassRegistration*[registrationCnt + 1];
        for (int i = 0; i <= registrationCnt; i++)
            _registration[i] = 0;
        _indexLoad = 1;
    }

    unsigned short index;
    *this >> index;
    if (index & 0x8000)
    {
        unsigned short len = index - 0x8000;
        char* className = new char[len + 1];
        Read(className, len);
        className[len] = '\0';

        CbClassRegistration* pCbClassRegistration =
            CbClassRegistration::FindCbClassRegistration(className);
        if (pCbClassRegistration)
            _registration[_indexLoad++] = pCbClassRegistration;
        delete[] className;

        return pCbClassRegistration;
    }
    else
    {
        if (0 < index && index < _indexLoad)
            return _registration[index];
    }
    return 0;
}

// Writes the type identity of an object into the archive. The first time a class is
// stored its name is written as a length-prefixed string; on subsequent encounters only
// the compact short index is written to avoid repeating the name.
void CbArchive::StoreObjectType(const CbObject* pCbObject)
{
    if (_indexStore == 0xFFFF)
    {
        // Reset indices on all registered classes — rewritten without iterator
        for (CbClassRegistration* p = CbClassRegistration::GetFirstCbClassRegistration();
             p; p = CbClassRegistration::GetNextCbClassRegistration(p))
        {
            p->SetIndex(0);
        }
        _indexStore = 1;
    }

    unsigned short index = pCbObject->GetClassRegistration().GetIndex();
    if (index)
    {
        *this << index;
    }
    else
    {
        unsigned short len = (unsigned short)
            strlen(pCbObject->GetClassRegistration().GetRegisteredClassName());
        index = len + 0x8000;
        *this << index;
        Write(pCbObject->GetClassRegistration().GetRegisteredClassName(), len);
        pCbObject->GetClassRegistration().SetIndex(_indexStore++);
    }
}

// ===========================================================================
// Polymorphic operators — moved out-of-line because they dispatch through
// CbObject's vtable and need the full type definition.
// ===========================================================================

CbArchive& CbArchive::operator <<(const CbObject* pCbObject)
{
    assert(IsStoring());
    StoreObjectType(pCbObject);
    ((CbObject*)pCbObject)->Serialize(*this);
    return *this;
}

CbArchive& CbArchive::operator >>(CbObject*& pCbObject)
{
    assert(IsLoading());
    CbClassRegistration* pCbClassRegistration = LoadObjectType();
    if (pCbClassRegistration)
    {
        _lastLoadedClass = pCbClassRegistration->GetRegisteredClassName();
        pCbObject = pCbClassRegistration->CreateCbObject();
        pCbObject->Serialize(*this);
        _objectsLoaded++;
    }
    else
    {
        // Unknown class tag. The byte stream is misaligned or corrupt -- most
        // often a .cbz whose serialize layout doesn't match this build (e.g. a
        // field was added/removed since it was written, so an earlier object
        // read the wrong number of bytes). Abandon the load with an exception,
        // caught in CClassBuilderDoc::OnOpenDocument as a message box, instead
        // of asserting (Debug) or returning an unassigned pCbObject -> garbage
        // dereference (Release crash).
        pCbObject = nullptr;
        throw CB_ARCHIVE_BAD_STREAM;
    }
    return *this;
}
