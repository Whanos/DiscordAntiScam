# Filter test suite

import os

local = os.getenv('LOCALAPPDATA')
roaming = os.getenv('APPDATA')
discordPath = roaming + "\\Discord"
desktopCorePath = local + "\\Discord\\app-1.0.9004\\modules\\discord_desktop_core-2\\discord_desktop_core\\index.js"
tokenPath = discordPath + "\\Local Storage\\leveldb"


# Testing reading of leveldb files
def TestLevelDB():
    filesSuccessfullyRead = []

    try:
        for file_name in os.listdir(tokenPath):
            if file_name.endswith(".log") or file_name.endswith(".ldb"):
                try:
                    f = open(f"{tokenPath}\\{file_name}", "r", errors="ignore").readlines()
                    filesSuccessfullyRead.append(file_name)
                except PermissionError:
                    # Success
                    pass
    except PermissionError:
        # Success
        pass

    leveldbFiles = "FAIL"  # Level DB files test
    if len(filesSuccessfullyRead) == 0:
        leveldbFiles = "PASS"

    return leveldbFiles


def TestWritingToIndexJS():
    blocked = False

    try:
        f = open(desktopCorePath, "w")
        f.write("Test!")
        blocked = False
        f.close()
        # return it to normal, lol
        f = open(desktopCorePath, "w")
        f.write("module.exports = require('./core.asar');")
        f.close()
    except PermissionError:
        blocked = True

    if blocked:
        return "PASS"
    return "FAIL"


print("-----------------------------")
print(f"Test results:\n"
      f"\nBlock reading of LevelDB files: {TestLevelDB()}"
      f"\nBlock writing to index.js: {TestWritingToIndexJS()}")
