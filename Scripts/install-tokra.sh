#!/bin/bash
# INSTALL TOK'RA GUARDIAN — ONE COMMAND

echo "IMPLANTING TOK'RA SYMBIOTE..."

mkdir -p .git/hooks
cp .git-hooks/tok-ra-pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit

echo "TOK'RA IMPLANTED — YOU ARE NOW PROTECTED"
echo "Run 'git commit' to test"