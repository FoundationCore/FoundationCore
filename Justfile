# Detect AUR helper (yay preferred, paru fallback)

aur := `if command -v yay &>/dev/null; then echo yay; elif command -v paru &>/dev/null; then echo paru; else echo ""; fi`

default: debug

# Install all system dependencies
install-deps:
    #!/usr/bin/env bash
    set -euo pipefail

    echo "==> Installing system packages..."
    sudo pacman -S --needed --noconfirm \
        clang \
        lldb \
        lld \
        libc++ \
        libc++abi \
        cmake \
        ninja

    echo "==> Installing conan..."
    if command -v yay &>/dev/null; then
        yay -S --needed --noconfirm conan-bin
    elif command -v paru &>/dev/null; then
        paru -S --needed --noconfirm conan-bin
    else
        echo "ERROR: No AUR helper found. Install yay or paru first."
        exit 1
    fi

# Check formatting without modifying files
format-check:
    find src include \( -name '*.c' -o -name '*.h' \) | \
        xargs clang-format --dry-run --Werror

# Apply formatting in-place
format:
    find src include \( -name '*.c' -o -name '*.h' \) | \
        xargs clang-format -i

# Run static analysis (requires a compilation database)
lint:
    find src -name '*.c' | \
        xargs clang-tidy \
            --config-file=.clang-tidy \
            -p build/conan-debug/compile_commands.json

# Build debug binary (via Conan + CMake)
debug:
    conan install . --profile ./.conan2/profiles/clang --build missing -s build_type=Debug
    cmake --preset conan-debug
    cmake --build --preset conan-debug

# Build release binary (via Conan + CMake)
release:
    conan install . --profile ./.conan2/profiles/clang --build missing -s build_type=Release
    cmake --preset conan-release
    cmake --build --preset conan-release

# Remove all build artifacts
clean:
    rm -rf build/
