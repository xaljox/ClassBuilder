// qt/CommentHeaderDialog.h -- Qt editor for a template comment header.
//
// One class serves both the *.h and the *.cpp comment-header dialogs (the
// MFC original had two near-identical classes, CCppHeaderDialog /
// CHHeaderDialog); the differences are passed in as constructor arguments.
#pragma once

#include <QDialog>

#include "QtCommentHeaderDialog.h" // Qt_UpdateAllHeadersFn

class QPlainTextEdit;

class CommentHeaderDialog : public QDialog
{
    Q_OBJECT
public:
    CommentHeaderDialog(const QString& windowTitle,
                        const QString& updateAllPrompt,
                        Qt_UpdateAllHeadersFn updateAll,
                        void* userData,
                        QWidget* parent = nullptr);

    QString comment() const;
    void    setComment(const QString& text);

private slots:
    void onUpdateAll();

private:
    QPlainTextEdit*       m_edit;
    QString               m_updateAllPrompt;
    Qt_UpdateAllHeadersFn m_updateAll;
    void*                 m_userData;
};
