#!/bin/bash

file_insert_before() {
	[ -z "$1" ] && exit # file name
	[ -z "$2" ] && exit # string before which to insert
	[ -z "$3" ] && exit # what to insert
	#sed -i "" "s/$2/$3$2/" "$1"
	# due to platform independence
	echo $(python -c "from file_utils import*; insert_before(\"$1\", \"$2\", \"$3\");")
}

file_append_line() {
	[ -z "$1" ] && exit # file name
	[ -z "$2" ] && exit # string to append at the end of the file
	echo "$2" >> "$1"
}

file_replace() {
	[ -z "$1" ] && exit # file name
	[ -z "$2" ] && exit # regex to find
	[ -z "$3" ] && exit # text to replace regex to
	#echo "s/$2/$3/g$4"
	sed -i -E "s/$2/$3/g$4" $1
	#echo $(python -c "from file_utils import*; replace(\"$1\", \"$2\", \"$3\");")
}

file_search() {
	[ -z "$1" ] && exit # file name
	[ -z "$2" ] && exit # regex to find

	#echo "in $1 find $2"
	local contents=$(<$1)
	local nl=$'\n'
	local rest=${contents#*$2}
	local res=$(( ${#contents} - ${#rest} - ${#2} ))
	[ $res -ge 0 ] && echo $res || echo "-1"
	#[[ $(cat $1) =~ .*$2* ]] && true || false
}

full_path() {
	[ -z "$1" ] && exit # input path
	[ -d "$1" ] && dir_full_path $1
	[ -f "$1" ] && file_full_path $1
}

dir_full_path() {
	[ -z "$1" ] && exit # directory path
	[ ! -d "$1" ] && exit
	
	local cur_dir=${PWD}
	cd "$1"
	echo ${PWD}
	cd $cur_dir
}

file_full_path() {
	[ -z "$1" ] && exit # file path
	[ ! -f "$1" ] && exit
	
	local file_dir=$(dirname "$1")
	local file_name=$(basename "$1")
	local cur_dir=${PWD}
	cd "$file_dir"
	echo ${PWD}/$file_name
	cd $cur_dir
}

rename() {
	[ -z "$1" ] && echo "No path specified" && exit # path
	[ ! -d "$1" ] && [ ! -f "$1" ] && echo "No file or directory exists in the given path '$1'" && exit 1
	[ -z "$2" ] && echo "No new name specified" && exit 2 # new name
	
	local b=$(dirname "$1")
	local new_path="$b/$2"
	if [ -d "$new_path" ] || [ -f "$new_path" ]; then
		echo "This name is already taken: '$new_path'"
		exit 3
	fi
	mv "$1" "$new_path"
}
