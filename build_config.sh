#!/bin/bash

source external_config.sh

buildFolderPrefix="Build"
extraArg=" -DDEPS=${HOME}/Projects"
extraArgWin=$extraArg
extraArgMac=$extraArg
logArg=" -DLOG_ON=ON -DLOG_LEVEL_DEBUG=ON"
