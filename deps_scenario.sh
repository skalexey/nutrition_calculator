#!/bin/bash

function deps_scenario()
{
    source dependencies.sh
    source deps_config.sh

    download_dependency "Utils" "$depsLocation" "git@github.com:skalexey/Utils.git"
    download_dependency "Networking" "$depsLocation" "git@github.com:skalexey/Networking.git"
    download_dependency "DataModelBuilder" "$depsLocation" "git@github.com:skalexey/DataModelBuilder.git"
    source "$depsLocation/DataModelBuilder/Core/deps_scenario.sh"
    source $depsLocation/Networking/netlib/external_config.sh
}

deps_scenario $@

