#!/usr/bin/env bash

set -euo pipefail

if (( $# != 0 )); then
    echo "Usage: $0" >&2
    exit 2
fi

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
project_dir="$(CDPATH= cd -- "$script_dir/.." && pwd)"
workspace_dir="$(CDPATH= cd -- "$project_dir/.." && pwd)"

build_dir="$project_dir/build"
skey_builder="$script_dir/agcore_skey_build"
flash_args="$build_dir/flash_args"
sdkconfig_header="$build_dir/config/sdkconfig.h"
partition_bin="$build_dir/partition_table/partition-table.bin"
partition_tool="$workspace_dir/esp-idf/components/partition_table/gen_esp32part.py"
cmake_cache="$build_dir/CMakeCache.txt"

if [[ ! -x "$skey_builder" ]]; then
    echo "Missing executable: $skey_builder" >&2
    exit 1
fi

for input in "$flash_args" "$sdkconfig_header" "$partition_bin" "$partition_tool" "$cmake_cache"; do
    if [[ ! -f "$input" ]]; then
        echo "Missing ESP-IDF build output: $input" >&2
        exit 1
    fi
done

project_name="$(sed -n 's/^CMAKE_PROJECT_NAME:STATIC=//p' "$cmake_cache" | head -n 1)"

if [[ ! "$project_name" =~ ^[A-Za-z0-9_.+-]+$ ]]; then
    echo "Project name not found in $cmake_cache" >&2
    exit 1
fi

python_bin=""

if [[ -f "$build_dir/CMakeCache.txt" ]]; then
    python_bin="$(sed -n 's/^PYTHON:UNINITIALIZED=//p' "$build_dir/CMakeCache.txt" | head -n 1)"
fi

if [[ -z "$python_bin" || ! -x "$python_bin" ]]; then
    echo "ESP-IDF Python interpreter not found; build the project first" >&2
    exit 1
fi

esptool_bin="$(dirname -- "$python_bin")/esptool"

if [[ ! -x "$esptool_bin" ]]; then
    echo "ESP-IDF esptool executable not found" >&2
    exit 1
fi

chip="$(sed -n 's/^#define CONFIG_IDF_TARGET "\([^"]*\)"$/\1/p' "$sdkconfig_header")"

if [[ -z "$chip" ]]; then
    echo "CONFIG_IDF_TARGET not found in $sdkconfig_header" >&2
    exit 1
fi

tmp_dir="$(mktemp -d "$build_dir/.fw_pack.XXXXXX")"

cleanup()
{
    rm -rf -- "$tmp_dir"
}

trap cleanup EXIT HUP INT TERM

partition_csv="$tmp_dir/partition-table.csv"
if ! "$python_bin" "$partition_tool" "$partition_bin" "$partition_csv" >/dev/null 2>&1; then
    echo "Unable to parse partition table: $partition_bin" >&2
    exit 1
fi

read -r eskey_offset eskey_size_text <<<"$(
    awk -F, '$1 ~ /^[[:space:]]*eskey[[:space:]]*$/ {
        gsub(/[[:space:]]/, "", $4)
        gsub(/[[:space:]]/, "", $5)
        print $4, $5
    }' "$partition_csv"
)"

if [[ -z "$eskey_offset" || -z "$eskey_size_text" ]]; then
    echo "eskey partition not found in $partition_bin" >&2
    exit 1
fi

case "$eskey_size_text" in
    *[Kk]) eskey_size=$(( ${eskey_size_text%?} * 1024 )) ;;
    *[Mm]) eskey_size=$(( ${eskey_size_text%?} * 1024 * 1024 )) ;;
    *)     eskey_size=$(( eskey_size_text )) ;;
esac

printf 'Device: '
if ! IFS= read -r device; then
    echo "Unable to read Device" >&2
    exit 1
fi
device="${device%$'\r'}"

if [[ ! "$device" =~ ^[0-9A-Fa-f]{4}_[0-9A-Fa-f]{4}_[0-9A-Fa-f]{4}_[0-9A-Fa-f]{4}$ ]]; then
    echo "Invalid Device; expected XXXX_XXXX_XXXX_XXXX" >&2
    exit 1
fi

release_name="${project_name}_${device}_release"
release_dir="$build_dir/$release_name"
release_stage="$tmp_dir/$release_name"
full_name="${project_name}_full.bin"
mkdir "$release_stage"

# 每次正式打包生成新的 SENC / ENC。
(
    cd "$tmp_dir"
    printf '%s\n' "$device" | "$skey_builder" >/dev/null
)

skey_bin="$tmp_dir/skey.bin"

if [[ ! -f "$skey_bin" ]]; then
    echo "agcore_skey_build did not generate skey.bin" >&2
    exit 1
fi

skey_size="$(stat -c '%s' "$skey_bin")"

if (( skey_size != eskey_size )); then
    echo "Invalid skey.bin size: $skey_size; eskey partition size is $eskey_size" >&2
    exit 1
fi

merged_bin="$release_stage/$full_name"
merge_log="$tmp_dir/esptool.log"

# ESP-IDF 已经在 flash_args 中提供 flash mode/freq/size
# 以及 bootloader、partition table、otadata、app 的地址。
if ! (
    cd "$build_dir"

    "$esptool_bin" --chip "$chip" merge-bin \
        --output "$merged_bin" \
        "@$flash_args" \
        "$eskey_offset" "$skey_bin"
) >"$merge_log" 2>&1; then
    echo "esptool merge-bin failed" >&2
    sed -n '1,200p' "$merge_log" >&2
    exit 1
fi

merged_size="$(stat -c %s "$merged_bin")"
eskey_end=$((eskey_offset + eskey_size))

if (( merged_size < eskey_end )); then
    echo "Merged image does not contain the complete eskey partition" >&2
    exit 1
fi

# OTA pack stage reserved. Do not create an OTA artifact until its header is defined.

if [[ -e "$release_dir" && ! -d "$release_dir" ]]; then
    echo "Release path is not a directory: $release_dir" >&2
    exit 1
fi

if [[ -d "$release_dir" ]]; then
    mv -f -- "$merged_bin" "$release_dir/$full_name"
else
    mv -- "$release_stage" "$release_dir"
fi

printf 'Generated:\nbuild/%s/\n└── %s\n' "$release_name" "$full_name"
