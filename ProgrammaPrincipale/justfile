set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]

format:
    rg --files | rg "(cpp|hpp|ino)" | % { clang-format -i $_ }
