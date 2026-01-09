#!/bin/bash
# Build script for LibrePods Windows (for CI/CD)

set -e

echo "================================================"
echo "LibrePods for Windows - Build Script"
echo "================================================"
echo

# Check if dotnet is installed
if ! command -v dotnet &> /dev/null; then
    echo "ERROR: .NET SDK not found!"
    echo "Please install .NET 8.0 SDK from https://dotnet.microsoft.com/download"
    exit 1
fi

echo "Checking .NET version..."
dotnet --version
echo

# Set configuration (Debug or Release)
CONFIG="${1:-Release}"

echo "Building LibrePods in $CONFIG configuration..."
echo

# Clean previous builds
echo "Cleaning previous builds..."
dotnet clean -c "$CONFIG"

# Restore NuGet packages
echo "Restoring NuGet packages..."
dotnet restore

# Build LibrePods.Core
echo
echo "Building LibrePods.Core..."
dotnet build LibrePods.Core/LibrePods.Core.csproj -c "$CONFIG"

# Build LibrePods.Windows
echo
echo "Building LibrePods.Windows..."
dotnet build LibrePods.Windows/LibrePods.Windows.csproj -c "$CONFIG"

# Publish for distribution
echo
echo "Publishing for distribution..."
dotnet publish LibrePods.Windows/LibrePods.Windows.csproj \
    -c "$CONFIG" \
    -r win-x64 \
    --self-contained false \
    -o publish/win-x64

echo
echo "================================================"
echo "Build completed successfully!"
echo
echo "Output location: publish/win-x64"
echo "================================================"
echo
