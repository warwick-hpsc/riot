#!/bin/bash

if [ $# == 0 ]
then
	dir=.
else
	dir=$1
fi

cnt=0
declare -A filedb

for i in ${dir}/*.filedb
do
	exec < $i
	while read line
	do
		fileid=`echo $line | awk '{ print $1 }'`
		filename=`echo $line | awk '{ print $2 }'`
	
		if [ -z ${filedb["$filename"]} ]
		then
			filedb["$filename"]=$cnt
			cnt=$(($cnt+1))
		fi
	done
done

echo -n "" > master.tmp

for j in ${!filedb[@]}
do
	echo -e ${filedb["$j"]}"\t"$j >> master.tmp
done

sort -n master.tmp > ${dir}/master.filedb

rm master.tmp
