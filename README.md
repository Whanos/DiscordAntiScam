# DiscordAntiScam
Stop getting your token grabbed.

The general idea is this is a MiniFilter, which intercepts file operations, and will block programs attempting to read any discord file or write to the directory (excluding kernel) that's not discord itself.\
There will be a frontend app that will be used to monitor the MiniFilter and alert the user if any programs tryna be sus

## that ntddk.h file

yeah look, I would use the one that comes with WDK, but it's bugged and keeps saying "no_init_all" errors unless you do

```cpp
#define no_init_all
```
in the file

that is why

all rights go to microsoft for that file
