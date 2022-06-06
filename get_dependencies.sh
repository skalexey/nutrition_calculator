#!/bin/bash

get_dependencies()
{
	local enterDirectory=${PWD}

	if [ ! -z "$1" ]; then 
		echo "Go to the source directory passed: '$1'"
		cd "$1" # go to the source directory passed
	fi

	local folderName=${PWD##*/}

	source log.sh

	local log_prefix="-- [${folderName} get_dependencies script]: "

	log "Check for dependencies" " -"

	if [ ! -f "deps_config.sh" ]; then
		log "No dependencies" " -"
		exit
	fi
	source deps_config.sh

	source deps_scenario.sh $@
	local retval=$?
	if [ $retval -ne 0 ]; then
		log_error "Error occured during the deps_scenario.sh execution " " -"
		cd "${enterDirectory}"
		exit 1
	fi

	cd "${enterDirectory}"
}

get_dependencies $@