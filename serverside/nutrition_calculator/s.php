<?php
	$file = $_GET['p'];

	if (file_exists($file)) {
		header('Content-Description: File Transfer');
		header('Content-Type: application/octet-stream');
		header('Content-Disposition: attachment; filename='.basename($file));
		header('Content-Transfer-Encoding: binary');
		header('Expires: 0');
		header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
		header('Pragma: public');
		header('Content-Length: '.filesize($file));
		header('Modification-Time: '.gmdate('D, d M Y H:i:s \G\M\T', filemtime($file)));
		ob_clean();
		flush();
		readfile($file);
		exit;
	}
?>
