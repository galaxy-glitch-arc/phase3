/bin/rm outfile.txt
touch outfile.txt

make clean

for i in $(seq -f "%02g" 0 25)
do
  make test$i
  echo starting test $i ....  >> outfile.txt
  echo >> outfile.txt
  echo  running test $i
  ./test$i >> outfile.txt 2>&1 3>&-
  echo >> outfile.txt
  rm test$i.o test$i
done