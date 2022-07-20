<?php
	function get_user_dir($u)
	{
		if (!isset($u) || strlen($u) == 0)
			return NULL;
		$users_dir = "../../../nc_u";
		$dir_name = md5($u);
		$path = "$users_dir/$dir_name";
		if (!file_exists($path))
		{
			mkdir($path, 0777, true);
			//TODO: remove this
			$f = fopen("$path/item_info.txt", "w");
			fwrite($f, "");
			$f = fopen("$path/input.txt", "w");
			fwrite($f, "");
		}
		return "$path";
	}
?>
