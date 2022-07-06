#!/usr/bin/bash
function setup_environment()
{
	ssh j70492510@j70492510.myjino.ru  << HERE
		echo "Setup environment"
		cd domains/skalexey.ru

		local project_dir="nutrition_calculator"
		[ ! -d "$project_dir" ] && mkdir "$project_dir"

		echo "Finished"
	HERE
}

setup_environment $@