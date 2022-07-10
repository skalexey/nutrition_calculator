
<?php
upload_file();

function upload_file(){
	$uploadTo = "uploads/";
	$allowFileType = array('jpg','png','jpeg','gif','pdf','doc','txt');
	$fileName = $_FILES['file']['name'];
	$tempPath=$_FILES["file"]["tmp_name"];
    echo "Temp path: $tempPath\n";
	$basename = basename($fileName);
	$originalPath = $uploadTo.$basename;
	$fileType = pathinfo($originalPath, PATHINFO_EXTENSION);
	if(!empty($fileName)){
	
	   if(in_array($fileType, $allowFileType)){
		 // Upload file to server
		 if(move_uploaded_file($tempPath,$originalPath)){
			echo $fileName." was uploaded successfully";
		   // write here sql query to store image name in database
		  
		  }else{
			echo 'File Not uploaded ! try again';
		  }
	  }else{
		 echo $fileType." file type not allowed";
	  }
   }else{
	 echo "Please Select a file";
   }
}

?>
