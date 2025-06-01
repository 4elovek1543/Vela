#!/bin/bash


f_clear=0
dst="/opt/"

while [[ "$#" -gt 0 ]]; do
	case $1 in
		-h|--help)
			echo "HELP MESSAGE:"
			echo "-c|--clear       to run clear installation"
			shift
			;;
		-c|--clear) 
            echo "Running clear install"
            f_clear=1
			shift
			;;
		*)
			echo "Неизвестный аргумент: $1"
            exit
            ;;
	esac
done

if [ $f_clear == 1 ]; then
    # delete previous version
fi

# read -p "Prompt: " res


