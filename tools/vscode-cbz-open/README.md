# ClassBuilder: open .cbz

Klik in VS Code op een `.cbz`-bestand en het opent in ClassBuilder, via de bestandskoppeling van het
systeem (macOS `open`, Windows `start`, Linux `xdg-open`). Het VS Code-tabblad sluit meteen weer.
Openen als gewoon bestand kan nog via rechtermuisklik > "Open With..." > "Text Editor".

Installeren: `tools/vscode-cbz-open/build.sh` maakt `cbz-open-1.0.0.vsix`, daarna
`code --install-extension cbz-open-1.0.0.vsix` (of in VS Code: Extensions > ... > Install from VSIX).
