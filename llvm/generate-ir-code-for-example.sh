#!/bin/bash

if [ -z "$1" ] ; then
	echo "Usage: $0 example.c" >&2
	exit 1
fi

set -e

EXAMPLE_FILE="$1"
DIR="$(dirname "$0")"
IPANEMA="$DIR/../private/ipanema"
IPANEMA_INCLUDES_SYSLAYER="$IPANEMA/include/syslayer/linux"
IPANEMA_INCLUDES_SIM="$IPANEMA/include/sim"
IPANEMA_INCLUDES="-I$IPANEMA_INCLUDES_SYSLAYER -I$IPANEMA_INCLUDES_SIM"
INSTALL_DIR="$DIR/_install"
LLVM_BIN="$INSTALL_DIR/llvm/bin"
CFLAGS_FOR_EXAMPLE="-DDOUBLE_FLOAT -D__POSIX__ -D_DSC_EVENTQUEUE -D__RESET -DEXEC_SEQ"
LLVM_PASSES="$DIR/llvm-passes"
LLVM_PASSES_INSTALL="$INSTALL_DIR/llvm-passes"
LLVM_PASSES_LIB="$LLVM_PASSES_INSTALL/lib/MehariLLVMPasses.so"

PARTITIONING_TARGET_FUNCTIONS="evalND evalS"
PARTITIONING_COUNT=2

TEMPLATE_DIR="$DIR/examples/templates"
MEHARI_SOURCES="$DIR/examples/mehari"

OUTPUT_DIR="$DIR/_output"
OUTPUT_GRAPH_DIR="$OUTPUT_DIR/graph"
PARTITIONING_RESULTS_DIR="$OUTPUT_DIR/partitioning"

TMP="$(mktemp -t "$(basename "$EXAMPLE_FILE" .c)".XXXXXXX.c)"

sed '/^\s*if\s*(\s*!\s*(\*status.*$/ d' "$EXAMPLE_FILE" > "$TMP"

"$LLVM_BIN/clang" $CFLAGS_FOR_EXAMPLE -S -emit-llvm $IPANEMA_INCLUDES "$TMP" -o "$EXAMPLE_FILE.1.ll"

"$LLVM_BIN/opt" -load "$LLVM_PASSES_LIB" \
	-add-attr-always-inline -inline-functions "evalParameterCouplings" \
	-S "$EXAMPLE_FILE.1.ll" > "$EXAMPLE_FILE.2.ll"

"$LLVM_BIN/opt" -always-inline -S "$EXAMPLE_FILE.2.ll" > "$EXAMPLE_FILE.3.ll"

mkdir -p "$PARTITIONING_RESULTS_DIR"

"$LLVM_BIN/opt" -load "$LLVM_PASSES_LIB" \
	-partitioning \
	-template-dir "$TEMPLATE_DIR" \
	-partitioning-functions "$PARTITIONING_TARGET_FUNCTIONS" \
	-partitioning-count $PARTITIONING_COUNT \
	-partitioning-output-dir "$PARTITIONING_RESULTS_DIR" \
	-S "$EXAMPLE_FILE.3.ll"
