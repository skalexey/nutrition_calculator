#!/bin/bash

process_dependences()
{
	if [ -z "$1" ]; then
		[ ! -d "$1" ] && log_error "Non-existent directory provided: '$1'" && return 2
	else
		local dir=$(full_path .)
	fi
	
	log_info "Check for dependencies in '$dir'" " -"
	# local enterDirectory=${PWD}
	# cd $dir

	if [ ! -f "$dir/deps_scenario.sh" ]; then
		log_success "No deps_scenario.sh found in '$dir'" " -"
		# [ ! -z "$enterDirectory" ] && cd "$enterDirectory"
		return 0
	fi
	source $dir/deps_scenario.sh $@
	local retval=$?
	if [ $retval -ne 0 ]; then
		log_error "Error occured during $dir/deps_scenario.sh execution " " -"
		# [ ! -z "$enterDirectory" ] && cd "$enterDirectory"
		return 1
	fi
}

get_dependencies()
{
	source log.sh
	
	local log_prefix="-- [${PWD##*/} ${BASH_SOURCE[0]}]: "
	
	source file_utils.sh
	if [ -z "$1" ] && "$(full_path .)" != "$(full_path "$1")" ]; then
		log_info "Go to the directory passed: '$1'"
		process_dependences $1
	else
		process_dependences .
	fi
}

get_dependencies $@