grammarSpec=src/grammar.pegjs
grammarFile=src/compiler/parser.mts

echo '' > $grammarSpec
for file in `ls src/grammar/*.pegjs`; do
  echo 'Using' $file
  cat $file >> $grammarSpec
done

peggy --plugin ./node_modules/ts-pegjs/dist/tspegjs.js --extra-options-file ./peg-config.json --allowed-start-rules Program -o $grammarFile --cache $grammarSpec
eslint --quiet $grammarFile --fix >> /dev/null
prettier -w $grammarFile