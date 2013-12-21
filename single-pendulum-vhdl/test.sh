#!/bin/bash

set -e

if [ -z "$1" -o "$1" == "*" -o "$1" == "work.all" ] ; then
	TESTS="work.test_float_conversion work.test_sin"
else
	TESTS="$*"
fi

# The Xilinx settings script is confused by our arguments, so we must remove them.
while [ -n "$1" ] ; do
	shift
done

[ -n "$XILINX_SETTINGS_SCRIPT" ] && source "$XILINX_SETTINGS_SCRIPT"


cd "$(dirname "$0")"

cat >run_test.tcl <<EOF
cd "$(realpath "$(dirname "$0")")"
run all
exit
EOF

for test in $TESTS ; do
	[ -e "isim.log" ] && rm isim.log
	fuse -incremental -prj all.prj -o test_sim $test && ./test_sim -intstyle ise -tclbatch run_test.tcl || exit $?

	if ! [ -e "isim.log" ] ; then
		echo "ERROR: ISim hasn't created logfile isim.log!" >&2
		exit 1
	fi

	if ! grep -q "Simulator is doing circuit initialization process" isim.log ; then
		# We could also check for "Finished circuit initialization process", but it seems that
		# this is not printed, if we abort the simulation.
		echo "There seams to be some problem with the simulation." >&2
		exit 1
	fi

	if grep -q "The simulator has terminated in an unexpected manner" isim.log ; then
		echo "There seams to be some problem with the simulation." >&2
		exit 1
	fi

	if ! grep -q "INFO: Simulator is stopped" isim.log ; then
		echo "There seams to be some problem with the simulation." >&2
		exit 1
	fi

	if grep -v "^\*\* Failure:\s*NONE\. End of simulation\." isim.log | grep -q "^\*\* Failure:" ; then
		echo "The simulation has been stopped by a fatal error!" >&2
		exit 1
	fi

	# We use grep without '-q', so the user will see the error messages again.
	if grep "^at [^:]*: Error: " isim.log ; then
		echo "There was at least one error during the test run!" >&2
		exit 1
	fi
done

echo "PASSED"