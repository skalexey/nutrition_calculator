#!/bin/bash

tmp="${PWD}"
THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$THIS_DIR"
./build.sh . -rcb release
./deploy.sh
cd "$tmp"
