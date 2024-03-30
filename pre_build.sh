#!/bin/bash

function pre_build()
{
	./update_cmake_modules.sh .
}

pre_build $@