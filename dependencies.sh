#!/bin/bash

download_dependency()
{
	local folderName=${PWD##*/}

	source log.sh
	local log_prefix="-- [${folderName} dependencies script]: "

	local dep_dir_name=$1
	local deps_path=${2}
	local deps_path="${deps_path//\~/$HOME}"
	local repo=$3

	log "Resolve dependency directory '${dep_dir_name}'" " --"

	if [[ ! -d "${deps_path}" ]]; then
		log "Dependencies directory '${deps_path}' does not exist. Try to create it..." " ---"
		mkdir -p "${deps_path}"
		local retval=$?
		if [ $retval -ne 0 ]; then
			log_error "Directory '${deps_path}' creation error" " ---"
			exit 1
		else
			log "Created" " ---"
		fi
	fi

	if [[ ! -d "$deps_path/$dep_dir_name" ]]; then
		log "Dependency directory '$dep_dir_name' does not exist. Download..." " ---"
		local cur_path=$(PWD)
		cd "$deps_path"
		git clone ${repo}
		cd "${cur_path}"
		local retval=$?
		if [ $retval -ne 0 ]; then
			log_error "Directory '${dep_name}' creation error" " ---"
			exit 1
		else
			log_success "Completed download of dependency '$dep_dir_name'" " ---"
		fi
	else
		log "Dependency '$dep_dir_name' is already downloaded" " ---"
	fi
}
