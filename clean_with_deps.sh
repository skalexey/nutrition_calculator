#!/bin/bash

function update_dir() {
    dir="$nutrition_calculator_deps/$1"
	[ ! -d "$dir" ] && echo "No dir exists: '$dir'" return 1
	cd "$dir"
	python clean.py full
}

function clean_with_deps()
{
	if [ "$1" == "dmbcore" ]; then
		update_dir DataModelBuilder/Core
		return 0
	fi
	source external_config.sh
	python clean.py full

	update_dir Utils
	update_dir Networking/netlib
	update_dir DataModelBuilder/Core
}

clean_with_deps $@