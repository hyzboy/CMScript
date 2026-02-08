#!/bin/bash
# Download WASI-SDK for Linux/macOS
# This script downloads the latest WASI-SDK release and extracts it to third_party/wasi

set -e

echo "========================================"
echo "WASI-SDK Download Script"
echo "========================================"
echo

# Configuration
WASI_VERSION="22"
WASI_FULL_VERSION="wasi-sdk-22.0"
TARGET_DIR="../third_party/wasi"

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     
        PLATFORM="linux"
        DOWNLOAD_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-22/${WASI_FULL_VERSION}-linux.tar.gz"
        ;;
    Darwin*)    
        PLATFORM="macos"
        DOWNLOAD_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-22/${WASI_FULL_VERSION}-macos.tar.gz"
        ;;
    *)          
        echo "ERROR: Unsupported operating system: ${OS}"
        exit 1
        ;;
esac

echo "Detected OS: ${PLATFORM}"
echo "Target directory: ${TARGET_DIR}"
echo "Download URL: ${DOWNLOAD_URL}"
echo

# Create target directory
mkdir -p "${TARGET_DIR}"

# Check if already downloaded
if [ -f "${TARGET_DIR}/bin/clang" ]; then
    echo
    echo "WASI-SDK appears to be already installed in ${TARGET_DIR}"
    echo
    read -p "Do you want to re-download and overwrite? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Keeping existing installation."
        # Verify and exit
        echo
        echo "Verifying installation..."
        "${TARGET_DIR}/bin/clang" --version
        echo
        echo "Installation verified!"
        exit 0
    fi
    echo
    echo "Removing existing installation..."
    rm -rf "${TARGET_DIR:?}"/*
fi

# Download WASI-SDK
TEMP_FILE="wasi-sdk-temp.tar.gz"

echo
echo "Downloading WASI-SDK ${WASI_VERSION} for ${PLATFORM}..."
echo "This may take a few minutes..."
echo

# Try curl first, then wget
if command -v curl &> /dev/null; then
    curl -L -o "${TEMP_FILE}" "${DOWNLOAD_URL}"
elif command -v wget &> /dev/null; then
    wget -O "${TEMP_FILE}" "${DOWNLOAD_URL}"
else
    echo "ERROR: Neither curl nor wget found. Please install one of them."
    exit 1
fi

echo "Download completed!"
echo

# Extract the archive
echo "Extracting WASI-SDK..."
echo

tar -xzf "${TEMP_FILE}" -C "${TARGET_DIR}" --strip-components=1

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: Failed to extract archive"
    echo "The file may be corrupted. Please try again."
    rm -f "${TEMP_FILE}"
    exit 1
fi

# Clean up
echo
echo "Cleaning up temporary files..."
rm -f "${TEMP_FILE}"

# Verify installation
echo
echo "Verifying installation..."
echo

if [ -f "${TARGET_DIR}/bin/clang" ]; then
    echo "Success! WASI-SDK installed successfully."
    echo
    echo "Verifying clang version:"
    "${TARGET_DIR}/bin/clang" --version
    echo
    echo "Installation complete!"
    echo
    echo "WASI-SDK is now available at: ${TARGET_DIR}"
    echo
    echo "Next steps:"
    echo "1. Compile C/C++ to .wasm: scripts/compile_to_wasm.sh"
    echo "2. Read the guide: docs/WASI_SDK_GUIDE.md"
else
    echo
    echo "ERROR: Installation verification failed."
    echo "clang not found in ${TARGET_DIR}/bin/"
    exit 1
fi

echo
echo "========================================"
echo "Installation successful!"
echo "========================================"
