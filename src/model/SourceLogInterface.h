// ClassBuilder/SourceLogInterface.h -- source-write log + progress sink.
//
// MFC-free and Qt-free abstract interface. The model's code-generation
// methods (DataModel::SaveAllFiles / SaveModifiedFiles, Class::WriteHFile /
// WriteCppFile / SourceCheck) receive a SourceLogInterface* and report
// progress and log lines through it. The Qt SaveSourceDialog implements it;
// any other consumer (console, test stub, pipe API) could too.
//
// In the model this is the extern class `SourceLogInterface` (renamed from
// the former MFC `CSaveSourceDialog` 2026-05-19).
#pragma once

#include "CbString.h"

class SourceLogInterface
{
public:
    virtual ~SourceLogInterface() {}

    virtual void AddLog(const CbString& log) = 0;
    virtual void AddLogError(const CbString& log) = 0;
    virtual void AddLogWarning(const CbString& log) = 0;
    virtual void StepProgress() = 0;
};
