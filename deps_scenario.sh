#!/bin/bash

function deps_scenario()
{
    source dependencies.sh
    source deps_config.sh

    download_dependency "Utils" "$depsLocation" "git@github.com:skalexey/Utils.git"
    download_dependency "Networking" "$depsLocation" "git@github.com:skalexey/Networking.git"

    source $depsLocation/Networking/netlib/external_config.sh
}

deps_scenario $@

