#!/usr/bin/bash
function setup_environment()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	cd "$THIS_DIR"
	source log.sh
	local log_prefix="[setup_environment]: "
	
	local local_project_dir_name="nutrition_calculator"

	source serverside_config.sh
	source ssh.sh

	ssh -p $ssh_port $ssh_user@$ssh_host << HERE
		echo "Check if in home directory"
		if [ -z \$(ls | grep domains) ]; then
			echo "Not a home directory. Exit"
			exit 1
		else
			echo "Home directory"
		fi
		if [ ! -d "$users_dir_name" ]; then
			echo "Create users directory '$users_dir_name'"
			mkdir $users_dir_name
		else
			echo "Users directory '$users_dir_name' already exists"
		fi
HERE

	[ $? -ne 0 ] && log_error "Error while checking if in home directory" && return 1

	local remote_dir="$domain_dir/$project_dir_name"

	if [ "$1" == "update" ] || [ "$1" == "-u" ] || [ "$1" == "sync" ]; then
		log_info "Update the project content..."
		ssh_copy "$local_project_dir_name" "$remote_dir" "$ssh_host" "$ssh_user" "$ssh_port" "$ssh_pass" -a
		source pull_php_includes.sh
		[ $? -eq 0 ] && log_info "Done" || ( log_error "Error while updating" && return 10)
	else
		log_info "Try to check if remote project directory '$remote_dir' exists"
		if ! ssh_exists "$remote_dir" "$ssh_host" "$ssh_user" "$ssh_port" "$ssh_pass"; then
			log_info "Remote project directory does not exist in '$remote_dir'. Let's create it!"
			[ $? -ne 0 ] && log_error "Error in ssh_exists occured" && return 1
			log_info "Create remote project directory '$remote_dir'"
			ssh_copy "$local_project_dir_name" "$remote_dir" "$ssh_host" "$ssh_user" "$ssh_port" "$ssh_pass"
			[ $? -ne 0 ] && log_error "Error while copying local directory '$local_project_dir_name' to remote by path '$remote_dir'" && return 1

			source pull_php_includes.sh

			log_success "Done"
		else
			log_success "Remote project directory already exists in '$remote_dir'"
			[ $? -ne 0 ] && log_error "Error in ssh_exists occured" && return 1
		fi
	fi
	# ssh j70492510@j70492510.myjino.ru  << HERE
	# 	echo "Setup environment"
	# 	cd domains/skalexey.ru

	# 	local project_dir="nutrition_calculator"
	# 	[ ! -d "$project_dir" ] && mkdir "$project_dir"

	# 	echo "Finished"
	# HERE
}

setup_environment $@