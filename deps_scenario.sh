#!/bin/bash

function deps_scenario()
{
    source dependencies.sh
    source deps_config.sh

    download_dependency "Utils" "$depsLocation" "git@github.com:skalexey/Utils.git"
}

deps_scenario $@

