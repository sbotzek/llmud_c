#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "Usage: $0 FUNCTION_NAME REL_PATH_TO_SOURCE.c" >&2
  exit 1
fi

# ==== CONFIGURE THESE ====
SRC_DIR="./src"         # where your .c files live
HEADER_DIR="./include"  # where your .h files live
# =========================

FUNC="$1"
SRC_REL="$2"
SRC="$SRC_DIR/$SRC_REL"

if [[ ! -f "$SRC" ]]; then
  echo "Error: source file not found: $SRC" >&2
  exit 1
fi

declare -a INCLUDED_HDRS
INCLUDE_PATHS=("$HEADER_DIR" "$SRC_DIR")

include_header() {
  local hdr="$1"
  for seen in "${INCLUDED_HDRS[@]}"; do
    [[ "$seen" == "$hdr" ]] && return
  done
  INCLUDED_HDRS+=("$hdr")

  # Dump header minus include-guards and pragma once
  awk '
    /^\s*#\s*(ifndef|define|endif)\b/ { next }
    /^\s*#\s*pragma\s+once/        { next }
    { print }
  ' "$hdr"
  echo

  # Recurse into its own local includes
  local dir; dir=$(dirname "$hdr")
  ( grep -E '^\s*#\s*include\s*"[^"]+"' "$hdr" || true ) | \
    sed -E 's/^\s*#\s*include\s*"([^"]+)".*/\1/' | \
    while read -r sub; do
      # try same directory
      if [[ -f "$dir/$sub" ]]; then
        include_header "$dir/$sub"
        continue
      fi
      # otherwise search include paths
      for p in "${INCLUDE_PATHS[@]}"; do
        if [[ -f "$p/$sub" ]]; then
          include_header "$p/$sub"
          break
        fi
      done
    done
}

########################################
# 1) LOCATE & PRINT FUNCTION BODY      #
########################################
start_line=$(
  grep -n -E "^[[:space:]]*.*\b${FUNC}[[:space:]]*\(.*\)[[:space:]]*\{" "$SRC" \
    | head -n1 | cut -d: -f1 || true
)
if [[ -z "$start_line" ]]; then
  echo "Error: implementation of '$FUNC' not found in $SRC" >&2
  exit 1
fi

echo "/* --- FUNCTION ${FUNC}() in $SRC_REL --- */"
awk -v start="$start_line" '
  NR < start { next }
  {
    print
    opens  += gsub(/\{/, "{")
    closes += gsub(/\}/, "}")
    if (opens > 0 && opens == closes) exit
  }
' "$SRC"
echo

########################################
# 2) PRINT RAW #INCLUDE LINES          #
########################################
echo "/* --- INCLUDES in $SRC_REL --- */"
grep -E '^\s*#\s*include\s*("[^"]+"|<[^>]+>)' "$SRC"
echo

########################################
# 3) DUMP LOCAL HEADER CONTENTS        #
########################################
echo "/* --- LOCAL HEADER CONTENTS (recursively) --- */"
# only the double-quoted includes
( grep -E '^\s*#\s*include\s*"[^"]+"' "$SRC" || true ) | \
  sed -E 's/^\s*#\s*include\s*"([^"]+)".*/\1/' | \
  while read -r hdr; do
    for p in "${INCLUDE_PATHS[@]}"; do
      if [[ -f "$p/$hdr" ]]; then
        include_header "$p/$hdr"
        break
      fi
    done
  done
echo

########################################
# 4) STRUCTS & PROTOTYPES BEFORE FUNC  #
########################################
echo "/* --- DECLARATIONS before $FUNC (line $start_line) --- */"
awk -v end="$start_line" '
  BEGIN { in_struct=0; brace=0 }
  NR < end {
    if (/^\s*struct[[:space:]]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*{/) {
      in_struct=1; brace = gsub(/\{/, "{") - gsub(/\}/, "}")
      print; next
    }
    if (in_struct) {
      print; brace += gsub(/\{/, "{") - gsub(/\}/, "}")
      if (brace == 0) { in_struct=0; print "" }
      next
    }
    if (/^[A-Za-z_].*\(.*\);\s*$/) {
      print
    }
  }
' "$SRC"
