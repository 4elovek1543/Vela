#!/bin/bash

name="Vela"

f_clear=0
dst="/opt/"
src="../../Vela"

private=0
username=$USER
gropname="vela"

hyprautoadd=0
hyprconf="~/.config/hypr/hyprland.conf"

while [[ "$#" -gt 0 ]]; do
	case $1 in
		-h|--help)
			echo " ========== HELP MESSAGE: =========="
            echo "-h|--help        show this message"
			echo "-c|--clear       run clear installation"
            echo "-d|--dst         set installation folder (default: /opt/)"
			# echo "-i|--info        show more info wihle intall"
            echo "-p|--private     can configurate app only if in group $gropname, default add it to current user"
            echo "-y|--hyprcof     define path to hyprland config to add necessary data automaticaly"
            exit
			;;
		-c|--clear) 
            echo "Running clear install"
            f_clear=1
			shift
			;;
        -d|--dst)
            if [ "$#" -lt 2 ]; then
                echo "Not enough arguments after -d option"
                exit 1
            fi
            dst=$2
            echo "Folder to install changet to: $dst" 
            shift 2
            ;;
        -i|--info)
            shift
            ;;
        -p|--private)
            private=1
            echo "Group $gropname will be added to current user: $USER"
            echo "Only membrs of this group will be able to redact confug files"
            shift
            ;;
        -y|--hyprconf)
            if [ "$#" -lt 2 ]; then
                echo "Not enough arguments after -y option"
                exit 1
            fi
            hyprautoadd=1
            param=$2
            if [ "$param" != "auto" ]; then
                hyprconf="$param"
            fi
            echo "Necessary data will be added to the end of $hyprconf"
            shift 2
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
    if [ -e "$name" ]; then
        sudo rm -R "$name"
    fi
    cd $curd
    if [ -e "/usr/bin/vela" ]; then
        sudo unlink "/usr/bin/vela"
    fi
fi

# copy sources
sudo cp -R $src $dst

# go to dst folder
dst+="$name"
cd $dst

# define accessibility
if [ $private == 1 ]; then
    sudo groupadd $gropname
    sudo usermod -aG $gropname $username
    sudo chown -R $username:$gropname "$dst/config/"
    sudo chmod -R g+rw "$dst/config/"
else 
    sudo chmod -R a+rw "$dst/config/"
fi

# cmake .. & make
cd "build"
sudo cmake .. -DCMAKE_BUILD_TYPE=Release
sudo make

# create log file 
sudo touch "$dst/build/vela.log"
sudo chmod a+w "$dst/build/vela.log"

# create link to ./Vela
execfile="$(pwd)/Vela"
sudo ln -s "$execfile" "/usr/bin/vela"


echo "============ Vela Launcher is ready to start ============"
echo "Folder:          $dst"
echo "Launch:          vela or $dst/build/Vela"
echo "Log:             $dst/build/vela.log"
echo "Child log:       /tmp/vela/*.log does not clear if run into debug mode"
echo "Config folder:   $dst/config/"


# add data to hyprconfig
hyprdata=(
    "windowrulev2 = float,title:^Vela Launcher$"
    "windowrulev2 = noborder,title:^Vela Launcher$"
)
if [ hyprautoadd == 1 ]; then
    echo "" >> $hyprconf
    for item in "${hyprdata[@]}"; do
        echo "$item" >> $hyprconf
    done
else
    echo ""
    echo "Please copy this and paste it to your hyprlang config:"
    echo ""
    for item in "${hyprdata[@]}"; do
        echo "$item"
    done
    echo ""
fi
