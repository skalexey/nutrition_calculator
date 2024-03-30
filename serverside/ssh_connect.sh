#!/bin/bash
function job() {
	local THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
	source "$THIS_DIR/serverside_config.sh"
	ssh -p $ssh_port $ssh_user@$ssh_host
}

job $@
