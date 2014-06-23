#! /bin/bash

BASEDIR=$(dirname $0)
ASS="$BASEDIR/src/arnold_driver/cornell.ass"
DRIVER="$BASEDIR/build/src/arnold_driver"

if [ -z "$ARNOLD_ROOT_DIR" ]; then
    echo "The ARNOLD_ROOT_DIR environment variable is not defined"
    exit
fi

python $ARNOLD_ROOT_DIR/bin/pykick -l $DRIVER $ASS -v 6 "$@"
