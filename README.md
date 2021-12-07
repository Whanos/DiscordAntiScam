# DiscordAntiScam
Stop getting your token grabbed.

The general idea is this is a MiniFilter, which intercepts file operations, and will block programs attempting to read any discord file or write to the directory (excluding kernel) that's not discord itself.\
There will be a frontend app that will be used to monitor the MiniFilter and alert the user if any programs tryna be sus
