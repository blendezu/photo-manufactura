#!/bin/bash
# Photo Manufactura - Dependency Setup Script
# Restores missing library headers and binaries from the main branch 
# to prevent build breakages after branch switches.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "📦 Restoring dependencies from main branch..."

cd "$PROJECT_ROOT"

# Check out the libs directory from main without switching branches
git checkout main -- libs/

echo "✅ Dependencies restored successfully."
echo "   - libs/Halide/include"
echo "   - libs/onnxruntime/include"
echo ""
echo "🚀 You can now proceed with your build."
