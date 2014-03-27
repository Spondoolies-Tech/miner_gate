#!/bin/sh

decoder="zabbix_reader"
sender="zabbix_sender -vv -z 127.0.0.1 -i-"
#modifier="sed -e \'s/^/%s /\'"

main(){
    # read files in log directory

    if [ -z "`ls $1`" ]; then
        echo "no files"
    else
    #for each file
    for f in `ls $1`; do
        echo "sending $1/$f" 
	# get host
	host=`echo "$f" | sed -e "s/_.*//"`
        # 1. send to zabbix_sender
	
        #modifier=`printf "$modifier" $host`; $decoder $1/$f | $modifier | $sender   # can't figure out the mistake here
        $decoder $1/$f | sed "s/^/$host /" | $sender 
        echo "done"
        echo "done $f" >> send_log
        # 2. delete
        rm $1/$f
    done;
    fi
    sleep 1
    main $1 # recurse infinitely
}

dir=$1
if [ -z $dir ]; then
    echo "usage: "$0" log_dir"
    exit
fi
main $dir
