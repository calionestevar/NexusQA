echo
echo "═══════════════════════════════════════════════════"
echo "       RIDE OUT WITH ME — ROHAN ANSWERS THE CALL"
echo "       Forth, and fear no darkness!"
echo "═══════════════════════════════════════════════════"
echo

# Auto-detect project
PROJECT_FILE=$(find . -maxdepth 1 -name "*.uproject" | head -n1)
if [ -z "$PROJECT_FILE" ]; then
    echo "No .uproject found! You must ride from your keep (project root)."
    exit 1
fi
echo "Banner raised: $PROJECT_FILE"

MODE="NEXUS"
if [[ "$1" == "-legacy" || "$1" == "--legacy" ]]; then
    MODE="LEGACY"
fi

echo
echo "[1/3] Forging arms — compiling Editor..."
./Engine/Build/BatchFiles/Linux/Build.sh UnrealEditor Development -project="$PROJECT_FILE" -waitmutex > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "■■■ THE FORGES HAVE FAILED — ROHAN FALLS ■■■"
    exit 1
fi

if [ "$MODE" = "LEGACY" ]; then
    echo
    echo "[LEGACY MODE] Riding with old banners — Asgard calls..."
    ./UnrealEditor "$PROJECT_FILE" -run=Asgard -NullRHI -nosound -log -unattended
else
    echo
    echo "[NEXUS MODE] The riders of Nexus charge forth..."
    ./UnrealEditor "$PROJECT_FILE" \
        -nullrhi -nosound -unattended -log \
        -ExecCmds="Nexus.Execute $*; Quit"
fi

RESULT=$?

# Palantír on failure
if [ $RESULT -ne 0 ]; then
    echo
    echo "THE PALANTÍR SEES — RECORDING THE FALL OF OUR FOES"
    ./UnrealEditor "$PROJECT_FILE" -game -run=PalantirCapture -NullRHI -log
fi

echo
echo "═══════════════════════════════════════════════════"
if [ $RESULT -eq 0 ]; then
    echo "           VICTORY! THE REALM IS SAFE!"
    echo "           Ride now! Ride for ruin and the world’s ending!"
else
    echo "           ■■■ DEFEAT — THE HORNS OF ROHAN FADE ■■■"
    echo "           Death! Death! Death!"
fi
echo "═══════════════════════════════════════════════════"
exit $RESULT