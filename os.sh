#!/bin/bash

is_windows() {
	if [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
		return 0
	else
		return 1
	fi
}

is_nix() {
	if [[ is_windows ]]; then
		false
	else
		true
	fi
}

is_mac() {
	[[ $OSTYPE =~ .*darwin* ]] && true || false
}