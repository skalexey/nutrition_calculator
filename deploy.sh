#!/bin/bash

deploy_dir="/usr/bin"
cp Build-cmake/Release/nutrition_calculator "$deploy_dir"
[ $? -ne 0 ] && echo " --- Errors during deploying at '$deploy_dir'" || echo " --- Deployed successfully at '$deploy_dir'"