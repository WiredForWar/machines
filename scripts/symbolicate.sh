#!/usr/bin/env bash
#
# Resolve the addresses in a crash report to functions and source lines.
#
# Usage:
#   scripts/symbolicate.sh <report.txt> <binary-or-symbol-file>
#
# Which symbol file, and which tool, depend on how the binary was built:
#
#   mingw64      machines.exe.debug, split out at build time
#                SYMBOLIZER=x86_64-w64-mingw32-addr2line
#   GCC/Clang    machines.debug, likewise
#                SYMBOLIZER=addr2line  (the default)
#   MSVC         machines.pdb, found automatically beside machines.exe, so pass
#                the .exe rather than the .pdb
#                SYMBOLIZER=llvm-symbolizer
#
# llvm-symbolizer reads DWARF as well as PDB, so it serves all three if having
# one tool is worth more than having the usual one.
#
# IMAGE_BASE=0x140000000 overrides the address the image was linked for, for the
# case where no objdump is at hand to read it out of the binary.
#
# A report holds runtime addresses, and the image was relocated when it loaded,
# so each address is put back where the linker had it: subtract the address the
# image loaded at, then add the address it was linked for. The first comes from
# the report -- the "Module base:" line on Windows, the executable's first
# mapping in the memory map on POSIX -- and the second from the binary.

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "usage: $0 <report.txt> <binary-or-symbol-file>" >&2
    exit 2
fi

report="$1"
binary="$2"
symbolizer="${SYMBOLIZER:-${ADDR2LINE:-addr2line}}"

# The two tools take different options and answer in different shapes.
case "$(basename "$symbolizer")" in
    llvm-symbolizer*) style=llvm ;;
    *)                style=addr2line ;;
esac

for f in "$report" "$binary"; do
    if [ ! -f "$f" ]; then
        echo "$0: no such file: $f" >&2
        exit 2
    fi
done

if ! command -v "$symbolizer" >/dev/null 2>&1; then
    echo "$0: $symbolizer not found; install binutils or LLVM, or set SYMBOLIZER" >&2
    exit 2
fi

# A report written on Windows has CRLF line endings, and this normally runs on
# Linux, where nothing strips them. Every pattern below would then fail to match
# its own end of line, and the script would report no frames in a file that
# plainly contains them. Normalising once beats quoting a stray carriage return
# into every expression.
normalised=$(mktemp)
trap 'rm -f "$normalised"' EXIT
tr -d '\015' < "$report" > "$normalised"
report="$normalised"

# The module base, if the report carries one.
base=$(sed -n 's/^Module base: 0x\([0-9a-fA-F]*\).*/\1/p' "$report" | head -n 1)

# On POSIX there is no such line, and the load address is the start of the
# executable's first mapping in the copied-in memory map instead.
if [ -z "$base" ]; then
    base=$(awk '/^--- Memory map ---/ { inmap = 1; next }
                inmap && $2 ~ /^r/ && $3 == "00000000" && $6 ~ /\// {
                    sub(/-.*/, "", $1); print $1; exit
                }' "$report")
fi

if [ -n "$base" ]; then
    base=$((16#$base))
else
    base=0
fi

# Subtracting the load address gives an offset into the image, and the tools
# want an address as the linker laid it out -- which is not the same number.
# A PE puts its code at ImageBase + offset, usually 0x140000000, and a
# non-relocatable ELF at its first LOAD vaddr; only a position-independent ELF
# links at zero and makes the two agree. Getting this wrong resolves every
# frame to "??", which reads exactly like missing symbols.
objdump="${OBJDUMP:-${symbolizer%addr2line}objdump}"

if ! command -v "$objdump" >/dev/null 2>&1; then
    objdump=objdump
fi

image_base="${IMAGE_BASE:-}"

if [ -z "$image_base" ]; then
    image_base=$("$objdump" -p "$binary" 2>/dev/null | awk '
        /^ImageBase/                  { print $2; exit }
        $1 == "LOAD" && $3 == "vaddr" { sub(/^0x/, "", $4); print $4; exit }
    ')
fi

if [ -n "$image_base" ]; then
    image_base=$((16#${image_base#0x}))
else
    image_base=0
    echo "Warning: could not read the image base from $binary; assuming 0" >&2
fi

printf 'Load address: 0x%x\n' "$base"
printf 'Image base:   0x%x\n' "$image_base"
echo

# Everything above the call stack is context worth keeping in the output.
sed -n '1,/^--- Call stack ---$/p' "$report"

in_stack=0
unresolved=0
total=0

while IFS= read -r line; do
    case "$line" in
        "--- Call stack ---")
            in_stack=1
            continue
            ;;
        "--- Memory map ---" | "=== End of report ===")
            in_stack=0
            continue
            ;;
    esac

    [ "$in_stack" -eq 1 ] || continue

    address=$(printf '%s\n' "$line" | sed -n 's/^ *0x\([0-9a-fA-F]*\) *$/\1/p')

    if [ -z "$address" ]; then
        continue
    fi

    runtime=$((16#$address))

    # An address below the load address belongs to a system library rather than
    # to this binary, and resolving it here would invent a confident wrong
    # answer.
    if [ "$base" -ne 0 ] && [ "$runtime" -lt "$base" ]; then
        printf '  0x%x  (outside this module)\n' "$runtime"
        continue
    fi

    linked=$((runtime - base + image_base))
    query=$(printf '0x%x' "$linked")

    if [ "$style" = llvm ]; then
        # llvm-symbolizer answers with the name and the location on separate
        # lines, one pair per inlined frame. Pairing them up reads the way the
        # other tool's output does.
        resolved=$("$symbolizer" --obj="$binary" --demangle --functions=linkage --inlines "$query" 2>/dev/null |
            awk 'NR % 2 { name = $0; next }
                 # An address in a system library resolves to nothing here, and
                 # must read as nothing rather than as a nameless frame -- the
                 # count of these is what says the symbols are wrong.
                 name == "" || name == "??" { next }
                 { printf "%s%s at %s", (shown++ ? "; " : ""), name, $0 }
                 END { print "" }' || true)
    else
        # -C demangles, -f names the function, -i expands the frames the
        # optimiser inlined away, and -p puts each on one line.
        resolved=$("$symbolizer" -C -f -p -i -e "$binary" "$query" 2>/dev/null || true)
    fi

    if [ -z "$resolved" ]; then
        resolved="??"
    fi

    case "$resolved" in
        '??'*) unresolved=$((unresolved + 1)) ;;
    esac

    total=$((total + 1))

    printf '  0x%x  %s\n' "$runtime" "$resolved"
done < "$report"

# Every frame failing is not a stack of unknown functions; it is the wrong file.
if [ "$total" -gt 0 ] && [ "$unresolved" -eq "$total" ]; then
    echo
    echo "Warning: nothing resolved. The debug information almost certainly does not" >&2
    echo "belong to the build this report came from -- check the report's Commit line" >&2
    echo "against the build the symbols came from." >&2
fi
