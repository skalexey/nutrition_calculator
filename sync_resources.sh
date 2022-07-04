#!/bin/bash

function sync_file()
{
	source log.sh
	local log_prefix="[sync_file]: "
	[ -z "$1" ] && log_error "No file name provided" && return 1 || local fname="$1"
	[ -z "$2" ] && log_error "No remote directory provided" && return 2 || local remote_dir="$2"

	if is_mac; then
		local temp_dir="/var/folders/x3/s5vc1vgs4132nd81mblxzhc40000gn/T"
	else
		local temp_dir="${HOME}/AppData/Local/Temp"
	fi

	log_info "Sync file '$fname' in '$temp_dir' with '$remote_dir'"

	local this_f="$temp_dir/$fname"
	local remote_f="$remote_dir/$fname"

	# [ ! -f "$this_f" ] && false

	# Git file is always newer after a pull regardless of its real change date
	# source file_utils.sh
	# local f1=$this_f
	# local f2=$remote_f
	# if file_newer "$this_f" "$remote_f"; then
	# 	log_info "Local is newer. Push it to the remote dir"
	# else
	# 	log_info "Remote is newer. Grab it to the local dir"
	# 	local f1=$remote_f
	# 	local f2=$this_f
	# fi
	
	local ret=false
	source input.sh
	if ask_user "Push local file? If you answer No - the remote version will replace the local one"; then
		cp "$this_f" "$remote_f"
		local r=$?
		[ $? -eq 0 ] && log_info "cp '$this_f' '$remote_f'" || log_error "Error while copying '$this_f' to '$remote_f'"
		local ret=true
	else
		$automation_dir/git/pull.sh "$tmp_dir"
		cp "$remote_f" "$this_f"
		[ $? -eq 0 ] && log_info "cp '$remote_f' '$this_f'" || log_error "Error while copying '$remote_f' to '$this_f'"
		local ret=false
	fi

	[ $? -ne 0 ] && log_error "Errors while syncing file '$fname'" && return 3 || log_success "Synced"

	$ret
}

function sync_resources()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	cd "$THIS_DIR"

	local scripts_dir="${HOME}/Scripts"
	source "$scripts_dir/automation/automation_config.sh"
	source "$automation_dir/tmp_repo/tmp_config.sh"
	source log.sh
	local log_prefix="[sync_resources]: "
	local tmp_dir="$project_dir"
	
	# local cur_dir="${PWD}"
	# cd "$THIS_DIR"
	
	# cd "$cur_dir"

	#$automation_dir/git/pull.sh "$tmp_dir"

	local commit=false
	if sync_file "item_info.txt" "$tmp_dir"; then
		commit=true
	fi
	if sync_file "input.txt" "$tmp_dir"; then
		commit=true
	fi
	if $commit; then
		$automation_dir/git/commit.sh "$tmp_dir" "Sync resources"
	fi

	[ $? -ne 0 ] && log_error "Error while delivering resources" || log_success "Done"
}

sync_resources $@
