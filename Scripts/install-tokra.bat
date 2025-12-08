@echo off
echo IMPLANTING TOK'RA SYMBIOTE...

mkdir .git\hooks 2>nul
copy .git-hooks\tok-ra-pre-commit .git\hooks\pre-commit >nul
echo TOK'RA IMPLANTED — YOU ARE NOW PROTECTED
echo Run 'git commit' to test
pause