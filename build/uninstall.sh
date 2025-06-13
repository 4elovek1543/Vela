#!/bin/bash


path="/usr/bin/Vela"


while [[ "$#" -gt 0 ]]; do
	case $1 in
		-h|--help)
			echo " ========== HELP MESSAGE: =========="
            echo "Delete version of vela and delete symlink to it"
            echo "Default folder: $path"
            echo "-h|--help        show this message"
            echo "-f|--lfolder     define if you want delete version not in $path"
            exit
			;;
        -f|--folder)
            if [ "$#" -lt 2 ]; then
                echo "Not enough arguments after -f option"
                exit 1
            fi
            path=$2
            echo "Folder to clear changet to: $path" 
            shift 2
            ;;
		*)
			echo "Неизвестный аргумент: $1"
            exit
            ;;
	esac
done

if [ -f "$path" ]; then
    sudo rm -R "$path"
    echo "Deleted $path"
fi

if [ -f "/usr/bin/vela" ]; then
    sudo unlink "/usr/bin/vela"
    echo "Deleted symlink"
fi

echo "Uninstall done"