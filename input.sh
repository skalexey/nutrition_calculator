#!/bin/bash

function ask_user() {
	source log.sh
	local log_prefix="[ask_user]: "
	
	[ -z "$1" ] && log_error "No question provided" && return 1 || question="$1"

	res=false

	while true; do
		read -p "$question (y/n) " yn
		case $yn in 
			[yY] ) res=true
				break;;
			[nN] ) res=false
				break;;
			* ) echo invalid response;;
		esac
	done

	$res
}
