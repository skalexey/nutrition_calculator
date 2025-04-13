#!/bin/bash

function job()
{
	local executable_name="nutrition_calculator"
	local build_dir_prefix="Build-cmake/"
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	cd "$THIS_DIR"
	source log.sh
	local log_prefix="[deploy.sh]: "
	local build_config="release"

	# iterate arguments case-insensitive
	for arg in "$@"; do
		arg=$(echo "$arg" | tr '[:upper:]' '[:lower:]')
		if [ "$arg" == "debug" ]; then
			log_info "debug argument passed"
			build_config=$arg
		fi
	done

	local build_dir_postfix=$(echo "$build_config" | awk '{print toupper(substr($0,1,1)) tolower(substr($0,2))}')
	local build_dir="$build_dir_prefix$build_dir_postfix"
	source os.sh

	if is_windows; then
		# Run deploy.ps1
		local deploy_folder="nutrition_calculator"
		source $THIS_DIR/automation_config.sh
		source $scripts_dir/include/file_utils.sh
		local programfiles_path=$(powershell.exe $(to_win_path "$scripts_dir/automation/windows/envar.ps1") "ProgramFiles" | tr -d '\r\n')
		local deploy_dir="$programfiles_path\\$deploy_folder"
		powershell.exe -ExecutionPolicy Bypass -File "deploy.ps1" "$executable_name" "$build_dir" "$deploy_dir" $build_config
		if [ $? -ne 0 ]; then
			log_error " --- Errors during deploying"
			return 1
		fi
		local deploy_dir_bash="$(to_nix_path "$deploy_dir")"
		export PATH="$PATH:$deploy_dir"
		return $?
	elif is_mac; then
		local deploy_dir="/Applications"
		local shell_config="$HOME/.zshrc"
	else
		local deploy_dir="/usr/bin"
		local shell_config="$HOME/.bashrc"
	fi

	# Append to the shell configuration file for macOS or Linux
	if ! grep -q "$deploy_dir" "$shell_config"; then
		echo "export PATH=\"$deploy_dir:\\$PATH\"" >> "$shell_config"
		log_info "Added '$deploy_dir' to PATH permanently in '$shell_config'"
	else
		log_info "'$deploy_dir' is already in PATH permanently in '$shell_config'"
	fi

	if [ $? -ne 0 ]; then
		log_error " --- Errors during initial phase"
		exit 1
	fi

	if [ ! -d "$deploy_dir" ]; then
		log_info "Creating directory '$deploy_dir'"
		mkdir -p "$deploy_dir"
	fi

	cp "$build_dir/$executable_name" "$deploy_dir"
	if [ $? -ne 0 ]; then
		log_error " --- Errors during deploying at '$deploy_dir'"
		exit 1
	else
		log_success " --- Deployed successfully $build_config at '$deploy_dir'"
	fi
}

job $@
