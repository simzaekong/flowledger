#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
output="${1:-dist/screenshots}"
mkdir -p "$output"
work="$(mktemp -d)"
device=""
cleanup() {
    if [[ -n "$device" ]]; then
        xcrun simctl shutdown "$device" 2>/dev/null || true
        xcrun simctl delete "$device" 2>/dev/null || true
    fi
    rm -rf "$work"
}
trap cleanup EXIT

xcrun simctl list -j > "$work/simulators.json"
python3 - "$work/simulators.json" > "$work/selection" <<'PY'
import json
import sys

with open(sys.argv[1]) as handle:
    info = json.load(handle)
runtimes = [item for item in info["runtimes"]
            if item["isAvailable"] and ".iOS-" in item["identifier"]
            and int(item["version"].split(".")[0]) >= 26]
phones = {item["name"]: item["identifier"] for item in info["devicetypes"]}
if not runtimes:
    sys.exit("No iOS 26 or newer simulator runtime is available")
chosen = max(runtimes, key=lambda item: tuple(map(int, item["version"].split("."))))
devices = {item["identifier"]: item["name"] for item in chosen.get("supportedDeviceTypes", [])
           if item.get("productFamily") == "iPhone"}
preferred = phones.get("iPhone 17")
phone = preferred if preferred in devices else next(iter(devices), None)
if phone is None:
    sys.exit("No compatible iPhone simulator profile was found")
print(chosen["identifier"], phone)
PY
read -r runtime phone < "$work/selection"
echo "Using $phone on $runtime"
device="$(xcrun simctl create FlowLedger-CI "$phone" "$runtime")"
app="$work/FlowLedger.app"
mkdir -p "$app"
cp App/Info.plist App/AppIcon*.png "$app/"
/usr/libexec/PlistBuddy -c 'Set :CFBundleSupportedPlatforms:0 iPhoneSimulator' "$app/Info.plist"
cp -R App/Web "$app/Web"
sdk="$(xcrun --sdk iphonesimulator --show-sdk-path)"
xcrun --sdk iphonesimulator clang -isysroot "$sdk" -target arm64-apple-ios16.4-simulator \
    -DFLOWLEDGER_SIMULATOR_PREVIEW=1 -fobjc-arc -fblocks -Wall -Wextra -Wno-unused-parameter \
    -framework UIKit -framework Foundation -framework WebKit -framework CoreGraphics \
    App/main.m -o "$app/FlowLedger"
python3 - "$app/PreviewLedger.json" <<'PY'
from datetime import datetime, timezone
from pathlib import Path
import json
import sys

today = datetime.now(timezone.utc).date().isoformat()
rows = [
    ("용돈", 120000, "allowance", "income"),
    ("아르바이트", 60000, "pay", "income"),
    ("점심", 27500, "food", "expense"),
    ("버스", 9400, "transport", "expense"),
    ("책", 16800, "shopping", "expense"),
]
sample = json.dumps([
    dict(id=f"preview-{i}", name=name, amount=amount, category=category, kind=kind, date=today)
    for i, (name, amount, category, kind) in enumerate(rows)
], ensure_ascii=False)
Path(sys.argv[1]).write_text(sample, encoding="utf-8")
PY
codesign --force --sign - "$app"

xcrun simctl boot "$device"
xcrun simctl bootstatus "$device" -b
xcrun simctl status_bar "$device" override --time "9:41" --dataNetwork wifi \
    --wifiMode active --wifiBars 3 --cellularMode active --cellularBars 4 \
    --batteryState discharging --batteryLevel 100 || true
xcrun simctl install "$device" "$app"
bundle_id="app.flowledger.ios"
for appearance in light dark; do
    xcrun simctl ui "$device" appearance "$appearance"
    xcrun simctl launch --terminate-running-process "$device" "$bundle_id" \
        -AppleLanguages '(ko)' -AppleLocale ko_KR
    sleep 5
    xcrun simctl io "$device" screenshot "$output/FlowLedger-$appearance.png"
done

data="$(xcrun simctl get_app_container "$device" "$bundle_id" data)"
for preview in calendar search; do
    if [[ "$preview" == search ]]; then
        args=(-previewSearch 식비)
    else
        args=(-previewCalendar yes)
    fi
    xcrun simctl launch --terminate-running-process "$device" "$bundle_id" \
        -AppleLanguages '(ko)' -AppleLocale ko_KR "${args[@]}"
    if [[ "$preview" == calendar ]]; then
        for ((attempt = 0; attempt < 90; attempt++)); do
            [[ -f "$data/tmp/calendar-ready" ]] && break
            sleep 1
        done
        if [[ ! -f "$data/tmp/calendar-ready" ]]; then
            echo "The calendar preview did not finish presenting" >&2
            exit 1
        fi
    fi
    sleep 5
    xcrun simctl io "$device" screenshot "$output/FlowLedger-$preview.png"
done
