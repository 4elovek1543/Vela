# Vela

This is a lightweight program to make easier to launch your own programs or scripts

This programm work with Hyprland on Arch Linux

#### Instalation:

1. Clone this repository to your PC
2. Go to folder build
3. Here you have 2 scripts: rebuils.sh and install.sh
   1. rebuild.sh build project inplace and run it
   2. install.sh install to your PC -> to launch app execute just "vela"

#### Setup:

Go to folder config to change all settings and content

style.css - style config to all app

main.yaml - main config file for app

###### How to define your own modules:

Choose, where you want to see it, then describe it in "modules" in needed part of app.

You need to describe row, column and path to .yaml config of your module here (see examples).

Then you need to describe config for your module: go to folder config/modules and create new .yaml file.

Here you need to describe name, icon (relative path to icon starts from "config/static/"), tooltip, action, and list of additional style classes, defined at style.css.

action - when you press module on your screen, app will execute action on the shell. Here you can use constants you define in app/constants in main config like $constant_name and \$script:script_name.sh to run script in config/scripts via bash.

Examples of all this were already presented in project.

You can change modules, scripts and static folders in main config (relative from root of project or absolute)

#### Enjoy this app!
