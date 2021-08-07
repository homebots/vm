rm tmp/*.bin || true

CWD=/home/espbuilder/project

xtensa_gcc() {
  docker run --rm -it -v $(pwd):$CWD darlanalves/homebots-sdk xtensa-lx106-elf-gcc $@
}

INCLUDE_PATH="-I/home/espbuilder/esp-open-sdk/sdk/include -I/home/espbuilder/esp-open-sdk/sdk/include/sdk -I $CWD/test -I $CWD/src"

for spec in $(find test -iname "$SPEC*.spec.cpp" -type f);
do
  echo "Compiling $spec"
  specName=$(basename $spec)
  bin=/tmp/$specName.bin

  xtensa_gcc $INCLUDE_PATH -std=gnu++11 -o $bin $CWD/$spec

  echo "Running $bin"
  [ -f $bin ] && chmod +x $bin && $bin
done