// Opent .cbz-bestanden in ClassBuilder in plaats van in VS Code: een "custom editor" voor *.cbz die
// het bestand doorgeeft aan het standaardprogramma van het systeem en zijn eigen tabblad weer sluit.
// macOS: open (ClassBuilder krijgt een "open bestand", ook als hij al draait), Windows: start via de
// bestandskoppeling, Linux: xdg-open.
const vscode = require('vscode');
const childProcess = require('child_process');

function openWithSystem(filePath)
{
    let command;
    let args;

    if (process.platform === 'darwin')
    {
        command = 'open';
        args = [filePath];
    }
    else if (process.platform === 'win32')
    {
        command = 'cmd';
        args = ['/c', 'start', '', filePath];
    }
    else
    {
        command = 'xdg-open';
        args = [filePath];
    }

    const child = childProcess.spawn(command, args, { detached: true, stdio: 'ignore', windowsHide: true });
    child.on('error', (error) =>
    {
        vscode.window.showErrorMessage(`ClassBuilder openen lukte niet: ${error.message}`);
    });
    child.unref();
}

class CbzEditorProvider
{
    openCustomDocument(uri)
    {
        return { uri, dispose() {} };
    }

    resolveCustomEditor(document, panel)
    {
        panel.webview.html = '<p style="font-family: sans-serif">Geopend in ClassBuilder.</p>';
        openWithSystem(document.uri.fsPath);

        // Tabblad meteen weer sluiten: het bestand is in ClassBuilder open
        setTimeout(() => panel.dispose(), 50);
    }
}

function activate(context)
{
    context.subscriptions.push(
        vscode.window.registerCustomEditorProvider('xaljox.cbzOpen', new CbzEditorProvider(),
                                                   { supportsMultipleEditorsPerDocument: true }));
}

function deactivate() {}

module.exports = { activate, deactivate };
