#!/bin/bash

source os.sh

if is_windows; then
    deploy_dir="/usr/bin"
else
    deploy_dir="/usr/local/bin"    	
fi

f=Build-cmake/Release/nutrition_calculator
[ ! -f "$f" ] && f=Build-cmake/nutrition_calculator
[ ! -f "$f" ] && echo "No build found" && exit


if is_linux; then
    sudo cp $f "$deploy_dir"
else
    cp $f "$deploy_dir"
fi
[ $? -ne 0 ] && echo " --- Errors during deploying at '$deploy_dir'" || echo " --- Deployed successfully at '$deploy_dir'"
