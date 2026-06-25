// qt/CommentHeaderDialog.cpp -- Qt comment-header editor + the MFC bridge.
//
// Pure Qt: no MFC / afxwin.h headers, so it compiles cleanly alongside the
// MFC sources. Replaces the MFC CCppHeaderDialog / CHHeaderDialog pair.

#include "CommentHeaderDialog.h"
#include "QtApp.h"

#include <QByteArray>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "CodeEditor.h"          // CodeEditor::codeFont()

CommentHeaderDialog::CommentHeaderDialog(const QString& windowTitle,
                                         const QString& updateAllPrompt,
                                         Qt_UpdateAllHeadersFn updateAll,
                                         void* userData,
                                         QWidget* parent)
    : QDialog(parent)
    , m_edit(new QPlainTextEdit(this))
    , m_updateAllPrompt(updateAllPrompt)
    , m_updateAll(updateAll)
    , m_userData(userData)
{
    setWindowTitle(windowTitle);
    setModal(true);

    // The header is C++ source text -- show it in the shared code-editor
    // font (Medium weight, crisp black) and do not wrap (the MFC editor
    // scrolled horizontally; ES_AUTOHSCROLL).
    m_edit->setFont(CodeEditor::codeFont());
    m_edit->setLineWrapMode(QPlainTextEdit::NoWrap);

    QPalette pal = m_edit->palette();
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::Text, Qt::black);
    m_edit->setPalette(pal);

    auto* updateAllBtn = new QPushButton(QStringLiteral("&Update All Headers"),
                                         this);
    updateAllBtn->setEnabled(m_updateAll != nullptr);
    connect(updateAllBtn, &QPushButton::clicked,
            this, &CommentHeaderDialog::onUpdateAll);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* bottom = new QHBoxLayout;
    bottom->addWidget(updateAllBtn);
    bottom->addStretch(1);
    bottom->addWidget(buttons);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_edit, 1);
    layout->addLayout(bottom);

    resize(620, 460);
    setMinimumSize(420, 280);
}

QString CommentHeaderDialog::comment() const
{
    return m_edit->toPlainText();
}

void CommentHeaderDialog::setComment(const QString& text)
{
    m_edit->setPlainText(text);
}

void CommentHeaderDialog::onUpdateAll()
{
    if (!m_updateAll)
        return;

    if (QMessageBox::question(this, windowTitle(), m_updateAllPrompt)
 == QMessageBox::Yes)
    {
        m_updateAll(m_userData);
    }
}

// --- MFC bridge (declared in QtCommentHeaderDialog.h) ----------------------

bool Qt_ShowCommentHeaderDialog(std::string& comment,
                                const char* windowTitle,
                                const char* updateAllPrompt,
                                Qt_UpdateAllHeadersFn updateAll,
                                void* userData,
                                void* ownerHwnd)
{
    Qt_EnsureApplication();

    CommentHeaderDialog dlg(QString::fromLocal8Bit(windowTitle),
                            QString::fromLocal8Bit(updateAllPrompt),
                            updateAll, userData);

    // The model stores headers with CRLF line endings; QPlainTextEdit works
    // in '\n'. Convert in on the way in, and back out on the way out, so the
    // model's CRLF convention is preserved (generated source needs CRLF).
    QString text = QString::fromLocal8Bit(comment.c_str());
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    dlg.setComment(text);

    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    QString edited = dlg.comment();
    edited.replace(QStringLiteral("\n"), QStringLiteral("\r\n"));
    const QByteArray bytes = edited.toLocal8Bit();
    comment.assign(bytes.constData(), static_cast<size_t>(bytes.size()));
    return true;
}
