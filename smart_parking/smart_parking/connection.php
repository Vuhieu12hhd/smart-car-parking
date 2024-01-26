<?php
	//for MySQLi OOP
	$conn = new mysqli('localhost', 'root', '', 'smart_parking');
	if($conn->connect_error){
	   die("Connection failed: " . $conn->connect_error);
	}
?>