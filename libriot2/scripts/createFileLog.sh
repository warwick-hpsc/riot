#!/bin/bash

if [ $# == 0 ]
then
	dir=.
else
	dir=$1
fi

echo -n "" > master.tmp

for i in ${dir}/*.log
do
	declare -A filedb
	
	exec < ${i}.filedb
	while read line
	do
		fileid=`echo $line | awk '{ print $1 }'`
		filename=`echo $line | awk '{ print $2 }'`
	
		matchingid=`grep -w $filename ${dir}/master.filedb | awk '{ print $1 }'`

		filedb["$fileid"]=$matchingid
	done

	# find and replace id with matching id
	gsub=""
	for j in ${!filedb[@]}
	do
		gsub=$gsub"gsub(/$j/, ${filedb["$j"]}, \$4); "
	done

	awk "BEGIN { OFS = \"\t\" } { $gsub; print }" $i >> master.tmp

	unset filedb
done

sort -n master.tmp > ${dir}/master.log

rm master.tmp
