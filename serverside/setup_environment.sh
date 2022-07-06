#!/usr/bin/bash
function setup_environment()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	cd "$THIS_DIR"
	source log.sh
	local log_prefix="[setup_environment]: "
	
	local local_project_dir_name="nutrition_calculator"

	source serverside_config.sh
	local remote_dir="$domain_dir/$project_dir_name"

	source ssh.sh

	if [ "$1" == "update" ] || [ "$1" == "-u" ] || [ "$1" == "sync" ]; then
		log_info "Update the project content..."
		ssh_copy "$local_project_dir_name" "$remote_dir" "$ssh_host" "$ssh_user" "$ssh_pass" -a
		[ $? -eq 0 ] && log_info "Done" || ( log_error "Error while updating" && return 10)
	else
		if ! ssh_exists "$remote_dir" "$ssh_host" "$ssh_user" "$ssh_pass" ; then
			[ $? -ne 0 ] && log_error "Error in ssh_exists occured" && return 1

			log_info "Create remote project directory '$remote_dir'"
			ssh_copy "$local_project_dir_name" "$domain_dir" "$ssh_host" "$ssh_user" "$ssh_pass"
			[ $? -ne 0 ] && log_error "Error while copying local directory '$local_project_dir_name' to remote by path '$remote_dir'" && return 1
			if [ "$project_dir_name" != "$local_project_dir_name" ]; then
				ssh_rename "$domain_dir/$local_project_dir_name" "$project_dir_name" "$ssh_host" "$ssh_user" "$ssh_pass"
				[ $? -ne 0 ] && log_error "Error while renaming local directory '$domain_dir/$local_project_dir_name' to '$project_dir_name'" && return 2			
			fi
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