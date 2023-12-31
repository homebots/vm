mdfile=opcodes.md

echo "# All opcodes" > $mdfile
echo "| code | byte |" >> $mdfile
echo "| -- | -- |" >> $mdfile

cat src/vm.hpp | grep 'const byte op_' | sed 's/const byte op_/|/' | sed 's/ = /\t|\t/' | sed 's/;/|/'  >> $mdfile

echo "# All value types" >> $mdfile
echo "| type | byte |" >> $mdfile
echo "| -- | -- |" >> $mdfile

cat src/vm.hpp | grep 'const byte vt_' | sed 's/const byte vt_/|/' | sed 's/ = /\t|\t/' | sed 's/;/|/'  >> $mdfile