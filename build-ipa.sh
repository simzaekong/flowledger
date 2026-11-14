#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
destination="${1:-$PWD/dist}"
mkdir -p "$destination"
destination="$(cd "$destination" && pwd)"
temporary="$(mktemp -d)"
trap 'rm -rf "$temporary"' EXIT
bundle="$temporary/Payload/FlowLedger.app"
mkdir -p "$bundle"

if [[ "$(uname -s)" == Darwin ]]; then
    sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
    xcrun --sdk iphoneos clang -isysroot "$sdk" -target arm64-apple-ios16.4 \
        -fobjc-arc -fblocks -Oz -Wall -Wextra -Wno-unused-parameter \
        -framework UIKit -framework Foundation -framework WebKit -framework CoreGraphics \
        -Wl,-adhoc_codesign App/main.m -o "$bundle/FlowLedger"
else
    clang="${CLANG:-clang}"
    linker="${LD64:-ld64.lld}"
    python3 Build/ios-imports.py "$temporary/imports"
    "$clang" -target arm64-apple-ios16.4 -fobjc-arc -fblocks -Oz \
        -Wall -Wextra -Werror -Wno-unused-parameter -c App/main.m -o "$temporary/main.o"
    "$linker" -arch arm64 -platform_version ios 16.4 26.0 -adhoc_codesign \
        -e _main "$temporary/main.o" "$temporary"/imports/*.tbd -o "$bundle/FlowLedger"
fi

cp App/Info.plist App/AppIcon*.png "$bundle/"
cp -R App/Web "$bundle/Web"
(cd "$temporary" && zip -q -r FlowLedger.ipa Payload)
mv "$temporary/FlowLedger.ipa" "$destination/FlowLedger.ipa"
printf '%s\n' "$destination/FlowLedger.ipa"
