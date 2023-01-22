#!/bin/bash

function deps_scenario()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source $THIS_DIR/dependencies.sh
    source $THIS_DIR/deps_config.sh

    download_dependency "Utils" "$depsLocation" "git@github.com:skalexey/Utils.git"
    download_dependency "Networking" "$depsLocation" "git@github.com:skalexey/Networking.git"
    download_dependency "DataModelBuilder" "$depsLocation" "git@github.com:skalexey/DataModelBuilder.git"
    source "$depsLocation/DataModelBuilder/Core/deps_scenario.sh"
    source "$depsLocation/Networking/netlib/external_config.sh"
    download_dependency "php_include" "$depsLocation" "git@github.com:skalexey/php_include.git"
}

deps_scenario $@

