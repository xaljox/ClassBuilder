// ClassBuilder/ParseLogInterface.h -- source-read / parse log + progress sink.
//
// MFC-free and Qt-free abstract interface. The model's source-reading code
// (DataModel::ReadAllFiles) and the flex/yacc parser (Read.y / Read.y.cpp)
// report progress and log lines through it. The Qt ReadSourceDialog
// implements it.
//
// In the model this is the extern class `ParseLogInterface` (renamed from
// the former MFC `CReadSourceDialog` 2026-05-19).
//
// Methods take `const char*` rather than CbString: the parser builds its
// log strings as MFC CString, the model side passes CbString -- both convert
// to `const char*` implicitly, so neither side needs adapting.
#pragma once

class ParseLogInterface
{
public:
    virtual ~ParseLogInterface() {}

    virtual void AddLog(const char* log) = 0;
    virtual void AddLogError(const char* log) = 0;
    virtual void AddLogWarning(const char* log) = 0;
    virtual void StepProgress() = 0;
};
