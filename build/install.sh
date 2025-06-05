#!/bin/bash

name="Vela"

f_clear=0
dst="/opt/"
src="../../Vela"

while [[ "$#" -gt 0 ]]; do
	case $1 in
		-h|--help)
			echo " ========== HELP MESSAGE: =========="
            echo "-h|--help        show this message"
			echo "-c|--clear       run clear installation"
            echo "-d|--dst         set installation folder (default: /opt/)"
			echo "-i|--info        show more info wihle intall"
            exit
			;;
		-c|--clear) 
            echo "Running clear install"
            f_clear=1
			shift
			;;
        -d|--dst)
            dst=$2
            echo "Folder to install changet to: $dst" 
            shift 2
            ;;
        -i|--info)
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
    curd=$(pwd)
    cd $dst
    sudo rm -R "$name"
    cd $curd
    sudo unlink "/usr/bin/vela"
fi

# cp sources
sudo cp -R $src $dst

# create file with path
dst+="$name"
cd $dst
# echo "$dst" | sudo tee "src/projectpath.txt"

# cmake .. & make
cd "build"
sudo cmake ..
sudo make

sudo touch "$dst/build/vela.log"
sudo chmod a+w "$dst/build/vela.log"


# create link to ./Vela
execfile="$(pwd)/Vela"
sudo ln -s "$execfile" "/usr/bin/vela"
