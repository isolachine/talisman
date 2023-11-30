#!/bin/bash

PROJ="research"

if [ -n "${VTENV}" ]; then
    if [ "${BASE}" = "${VTENV}" -a "${PROJ}" = "${VTENV_NAME}" ]; then
        echo "${VTENV_NAME} Virtual Environment is already active."
    else
        echo "There is already ${VTENV_NAME} Virtual Environment activated."
        echo "(at ${VTENV})"
        echo "Deactivate it first (using command 'deactivate_vtenv'), to activate"
        echo "test environment."
    fi
    return 1
fi

if [ "$(uname)" == "Darwin" ]; then
	export READLINK="greadlink"
	export EXT="dylib"
	# export CPPFLAGS="-g -stdlib=libstdc++" # Mac 10.14 removed libstdc++
        export CPPFLAGS="-g -I/Library/Developer/CommandLineTools/usr/include/c++/v1"
	export LDFLAGS="-stdlib=libstdc++"
else
	export READLINK="readlink"
	export EXT="so"
	export CPPFLAGS="-g"
	export LDFLAGS=
fi

export TEST=$(dirname $($READLINK -f env.sh))
export BASE=$(dirname $TEST)
export LLVMBIN="$BASE/Debug+Asserts/bin"
export LLVMLIB="$BASE/Debug+Asserts/lib"
export LLVMSRCLIB="$BASE/lib"
export BENCHMARKS="$TEST/benchmarks"
export BBInfo="$TEST/instrumentation/BBInfo"
export TMP="$TEST/tmp"
export PATH="${LLVMBIN}:$PATH"
export VTENV="${BASE}"
export VTENV_NAME="${PROJ}"

# echo "Activating ${VTENV_NAME} Virtual Environment (at ${VTENV})."
# echo ""
# echo "To exit from this virtual environment, enter command 'deactivate_vtenv'."

export "VTENV_PATH_BACKUP"="${PATH}"
export "VTENV_PS1_BACKUP"="${PS1}"

deactivate_vtenv() {
    echo "Deactivating ${VTENV_NAME} Virtual Environment (at ${VTENV})."
    echo "Restoring previous environment settings."

    export "PATH"="${VTENV_PATH_BACKUP}"
    unset -v "VTENV_PATH_BACKUP"
    export "PS1"="${VTENV_PS1_BACKUP}"
    unset -v "VTENV_PS1_BACKUP"

    unset -v VTENV
    unset -v VTENV_NAME
    unset -v LLVMBIN
    unset -v LLVMLIB
    unset -v BENCHMARKS
    unset -v BBInfo
    unset -v TMP
    unset -v READLINK
    unset -v EXT
    unset -v CPPFLAGS
    unset -v LDFLAGS
    unset -f deactivate_vtenv

    if [ -n "$BASH" -o -n "$ZSH_VERSION" ]; then
        hash -r
    fi
}

export PATH="${LLVMBIN}:${PATH}"
export PS1="[${VTENV_NAME}]${PS1}"

if [ -n "$BASH" -o -n "$ZSH_VERSION" ]; then
    hash -r
fi

source ./autocmp.sh
