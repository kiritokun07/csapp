#!/usr/bin/env bash
# Data Lab build tools - run inside WSL Ubuntu after first-time setup
set -euo pipefail

LAB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$LAB_DIR"

echo "=== Installing build tools ==="
sudo apt-get update
sudo apt-get install -y build-essential gcc-multilib make

echo "=== Building btest, fshow, ishow ==="
make clean || true
make

echo "=== Checking dlc ==="
chmod +x dlc 2>/dev/null || true
if ./dlc bits.c; then
    echo "dlc: OK (no rule violations in current bits.c)"
else
    echo "dlc: reported issues (expected until you finish bits.c)"
fi

echo "=== Running btest (expect failures on placeholder code) ==="
./btest -g || true

echo ""
echo "Done. Daily workflow:"
echo "  cd $LAB_DIR"
echo "  # edit bits.c"
echo "  make btest && ./btest -f <function>"
echo "  ./dlc -e bits.c"
