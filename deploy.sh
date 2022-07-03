#!/bin/bash

source os.sh

if is_mac; then
	deploy_dir="/usr/local/bin"
else
	deploy_dir="/usr/bin"
fi

cp Build-cmake/Release/nutrition_calculator "$deploy_dir"
[ $? -ne 0 ] && echo " --- Errors during deploying at '$deploy_dir'" || echo " --- Deployed successfully at '$deploy_dir'"