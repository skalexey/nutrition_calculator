#!/bin/bash

function ssh_exists() {
	[ -z "$1" ] && echo "[ssh_exists]: No path provided" && return 1 || local path="$1"
	[ -z "$2" ] && echo "[ssh_exists]: No host provided" && return 2 || local host="$2"
	[ -z "$3" ] && echo "[ssh_exists]: No user name provided" && return 3 || local user="$3"
	[ -z "$4" ] && echo "[ssh_copy]: No port provided. Will use 22 by default" && return 4 || local port_arg=" -p $4"

	local USE_IP="-o StrictHostKeyChecking=no $user@$host"
	
	local ssh_pass=""
	if [ ! -z "$5" ]; then # pass
		local ssh_pass="sshpass -p $5"
	fi

	# if $ssh_pass ssh $USE_IP $ssh_port stat $path \> /dev/null 2\>\&1; then
	if ssh$port_arg $user@$host stat $path \> /dev/null 2\>\&1; then
		[ $? -ne 0 ] && echo "[ssh_exists]: Error while using sshpass" && false && return 4
		true
	else
		false
	fi
}

function ssh_copy() {
	[ -z "$1" ] && echo "[ssh_copy]: No local path provided" && return 1 || local local_path="$1"
	[ -z "$2" ] && echo "[ssh_copy]: No remote path provided" && return 2 || local remote_path="$2"
	[ -z "$3" ] && echo "[ssh_copy]: No host provided" && return 3 || local host="$3"
	[ -z "$4" ] && echo "[ssh_copy]: No user name provided" && return 4 || local user="$4"
	[ -z "$5" ] && echo "[ssh_copy]: No port provided. Will use 22 by default" && return 5 || local port_arg=" -P $5"

	local copy_contents_modifier=""
	for arg in "$@"; do
		[ "$arg" == "-a" ] && echo "-a option passed. Copy contents" && local copy_contents_modifier="/."
	done

	if [ -d "$local_path" ]; then
		local cmd="scp$port_arg -r"
	elif [ -f "$local_path" ]; then
		local cmd="scp$port_arg"
	else
		echo "[ssh_copy]: Not existent local path provided"
		return 4
	fi

	$cmd "$local_path$copy_contents_modifier" "$user@$host:$remote_path"
}

function ssh_rename()
{
	[ -z "$1" ] && echo "[ssh_rename]: No path provided" && return 1 || local path="$1"
	[ -z "$2" ] && echo "[ssh_rename]: No new name provided" && return 2 || local new_name="$2"
	[ -z "$3" ] && echo "[ssh_rename]: No host provided" && return 3 || local host="$3"
	[ -z "$4" ] && echo "[ssh_rename]: No user name provided" && return 4 || local user="$4"
	[ -z "$5" ] && echo "[ssh_rename]: No port provided. Will use 22 by default" && return 5 || local port_arg=" -p $5"

	if [ ! -z "$6" ]; then # pass
		local ssh_pass="sshpass -p $6"
	fi

	local containing_dir=$(dirname $path)
	echo "Containing dir of '$path': '$containing_dir'"
	$shh_pass ssh$port_arg $user@$host mv "$path" "$containing_dir/$new_name"
}


