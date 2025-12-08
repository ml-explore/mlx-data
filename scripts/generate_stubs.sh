#!/bin/bash
# Generate type stubs for mlx.data._c using pybind11-stubgen
# Run this script after building the package to regenerate stubs

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

# Check if virtual environment exists
if [ ! -d ".venv" ]; then
    echo "Error: Virtual environment not found. Please create one first:"
    echo "  python3 -m venv .venv"
    echo "  source .venv/bin/activate"
    echo "  pip install -e ."
    exit 1
fi

# Activate virtual environment
source .venv/bin/activate

# Check if pybind11-stubgen is installed
if ! command -v pybind11-stubgen &> /dev/null; then
    echo "Installing pybind11-stubgen..."
    pip install pybind11-stubgen
fi

# Check if the module is importable
if ! python -c "from mlx.data._c import Buffer" 2>/dev/null; then
    echo "Error: mlx.data._c module not found. Please install the package first:"
    echo "  pip install -e ."
    exit 1
fi

echo "Generating stubs for mlx.data._c..."

# Create temporary directory for stubs
TEMP_STUBS=$(mktemp -d)
trap "rm -rf $TEMP_STUBS" EXIT

# Generate stubs
pybind11-stubgen mlx.data._c -o "$TEMP_STUBS" \
    --ignore-invalid-expressions "mlx::data::.*" \
    --ignore-invalid-identifiers "typing_extensions.Buffer" \
    --ignore-unresolved-names "typing_extensions.Buffer" \
    --enum-class-locations "LoadAudioInfo:mlx.data._c.LoadAudioInfo" \
    --enum-class-locations "TokenizeMode:mlx.data._c.TokenizeMode"

# Copy stubs to the package
STUB_DEST="$PROJECT_ROOT/python/mlx/data/_c"
mkdir -p "$STUB_DEST"
cp "$TEMP_STUBS/mlx/data/_c/__init__.pyi" "$STUB_DEST/"
cp "$TEMP_STUBS/mlx/data/_c/core.pyi" "$STUB_DEST/"

# Fix naming conflict: typing_extensions.Buffer -> Buffer
# (Python's typing_extensions.Buffer is a different type than our Buffer class)
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' 's/typing_extensions\.Buffer/Buffer/g' "$STUB_DEST/__init__.pyi"
else
    sed -i 's/typing_extensions\.Buffer/Buffer/g' "$STUB_DEST/__init__.pyi"
fi

# Ensure py.typed markers exist
touch "$PROJECT_ROOT/python/mlx/data/py.typed"
touch "$STUB_DEST/py.typed"

echo "✅ Stubs generated successfully!"
echo "   - $STUB_DEST/__init__.pyi"
echo "   - $STUB_DEST/core.pyi"
echo ""
echo "To verify, run:"
echo "  pyright --verifytypes mlx.data._c"
