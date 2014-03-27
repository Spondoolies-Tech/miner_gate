#!/bin/sh
# create dummy log files, using fakewriter
# takes arumwent of host basename, host coutn, and rfequency
writer=fakewriter
dir="Logs/"

main(){
	date=`date +%F_%H%M%S`
	for i in `seq 1 $2`; do
		file=$1$i"_"$date".log"
		echo $file
		$writer $dir$file
	done


	sleep $3	
	main $1 $2 $3
}


if [ -z $3 ]; then
	printf "usage: $0 hosts_basname host_count create_frequency"
	exit 1
fi
main $1  $2  $3
