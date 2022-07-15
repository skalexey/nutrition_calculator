#!/bin/bash

get_dependencies()
{
	source log.sh

	local enterDirectory=${PWD}
	local log_prefix="-- [$1 get_dependencies script]: "

	if [ ! -z "$1" ]; then 
		
		log "Go to the source directory passed: '$1'"
		cd "$1" # go to the source directory passed
	fi

	local folderName=${PWD##*/}

	log "Check for dependencies" " -"

	if [ ! -f "deps_config.sh" ]; then
		log "No dependencies" " -"
		cd "${enterDirectory}"
		return 0
	fi
	source deps_config.sh

	source deps_scenario.sh $@
	local retval=$?
	if [ $retval -ne 0 ]; then
		log_error "Error occured during the deps_scenario.sh execution " " -"
		cd "${enterDirectory}"
		return 1
	fi

	cd "${enterDirectory}"
}

get_dependencies $@