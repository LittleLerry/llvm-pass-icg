clang -S -emit-llvm main.c -o main.ll
clang -fpass-plugin=`echo build/icg/icgPass.*` main.c -o main

opt -passes=dot-callgraph main.ll
mv main.ll.callgraph.dot i.dot

dot i.dot -Tpng -o before_iteration.png

for i in {1..10}
do
	random_number=$((1 + $RANDOM % 5))
	echo $random_number > input.txt
	echo "input:$random_number"
	./main < input.txt | python3 cg_iterator.py -ig i.dot -og o.dot
	rm -f i.dot
	mv o.dot i.dot
	# debug only
	dot i.dot -Tpng -o after_iteration.png   
	echo "Iteration $i done"
done

echo "Iteration done. Please find before_iteration.png and after_iteration.png"
 
dot i.dot -Tpng -o after_iteration.png      
