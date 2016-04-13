#!/bin/bash
room=127
maxmachines=37
usage="\e[36;01mUsage:\e[0m ./run.sh nnodes npernode executable [\"args\"]\n"
nodes=$1
if [ -z "$nodes" ]
    then
        echo -e "\e[31;01mError:\e[0m Number of nodes expected."
        echo -e $usage
        exit
fi
pprnode=$2
if [ -z "$pprnode" ]
    then
        echo -e "\e[31;01mError:\e[0m Number of processes per node expected"
        echo -e $usage
        exit
fi
executable=$3
if [ -z "$executable" ]
    then
        echo -e "\e[31;01mError:\e[0m Executable name expected"
        echo -e $usage
        exit
elif [ ! -f "$executable" ]
    then
        echo -e "\e[31;01mError:\e[0m Executable $executable does not exist"
        exit
fi
args=$4

hosts=""
computer=1
counter=0
maxnodes=$(expr $(expr $nodes + $(expr $pprnode - 1) ) / $pprnode )
while [[ counter -le maxnodes ]] && [[ computer -le maxmachines ]]
do
    format="$(printf $room"x%02d" ${computer} )"
    openconn="$(ping -s 0 -c 1 $format)"
    if [[ $openconn == *"1 received"* ]]
        then
            if [ -n "$SSH_VERIFICATION" ]
                then
                    temp="$(ssh -o StrictHostKeyChecking=no $format "pwd")"
            fi
            hosts=$hosts$format","
            ((counter++))
        else
            if [ -n "$DEBUG_MPI_HOSTS" ]
                then
                    echo "Host $format is unavailable."
            fi
    fi
    ((computer++))
done
if [ $counter -lt $maxnodes ]
    then
        echo "Error: Unable to connect to the required number of hosts"
        echo "Required hosts: $maxnodes"
        echo "Hosts found:    $counter"
        exit
fi

mpirun -H $hosts -n $nodes -map-by ppr:$pprnode:node $executable $args
