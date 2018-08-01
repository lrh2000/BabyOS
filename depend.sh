#! /bin/sh

if [ $# -lt 3 ] ;then
  echo "Usage:"
  echo "./depend.sh output_file input_file compiler compiler_flags"
  exit -1
fi

OUT_FILE="$1"
OUT_DIR=`dirname $OUT_FILE`
IN_FILE="$2"
IN_DIR=`dirname $IN_FILE`
mkdir -p $OUT_DIR
shift 2

$@ -MM -MG $IN_FILE | sed "s@^\(.*\)\\.o:@$OUT_DIR/\\1.o $OUT_DIR/\\1.d:@" > $OUT_FILE
