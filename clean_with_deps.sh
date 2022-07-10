#!/bin/bash

function clean_with_deps()
{
	source external_config.sh
	python clean.py full
	cd ${nutrition_calculator_deps}/Utils
	python clean.py full
	cd ${nutrition_calculator_deps}/Networking/netlib
	python clean.py full
}

clean_with_deps $@