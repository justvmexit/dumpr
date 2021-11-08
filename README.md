# dumpr
A process dumper written in C++ 17 with Windows Driver Kit using ioctl for communication between kernel-mode <-> user-mode even if handles are stripped.

![img](https://cdn.discordapp.com/attachments/895206720702386217/907260039448891422/unknown.png)

# usage
before using dumper.exe you have to load the driver using your preferred mapper.
[kdmapper](https://github.com/TheCruZ/kdmapper), [WindowsD](https://github.com/katlogic/WindowsD/releases/tag/v2.2), [gdrv-loader](https://github.com/fengjixuchui/gdrv-loader)
and then it's just
`dumper.exe notepad.exe` or `dumper.exe EscapeFromTarkov.exe`

# coding style
before anyone go and judge me i mix camel case and snake case because Windows Kernel stuff just matches camel case while snake case is more my personal coding style.

# dependencies
requires Visual Studio 2019
requires Windows Driver Kit (WDK)
requires a working computer