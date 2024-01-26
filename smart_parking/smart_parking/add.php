<?php
	session_start();
	include_once('connection.php');

	if(isset($_POST['add'])){
		$full_name = $_POST['full_name'];
		$birthday = $_POST['birthday'];
		$apartment_number = $_POST['apartment_number'];
		$UID = $_POST['UID'];
		$license_plate = $_POST['license_plate'];
		$email = $_POST['email'];
		$phone_number = $_POST['phone_number'];
		$expiration_date = $_POST['expiration_date'];
		$sql = "INSERT INTO `users` (`full_name`, `birthday`, `apartment_number`, `UID`, `license_plate`, `email`, `phone_number`, `expiration_date`) VALUES 
		('$full_name', '$birthday', '$apartment_number', '$UID', '$license_plate', '$email', '$phone_number', '$expiration_date')";

		//use for MySQLi OOP
		if($conn->query($sql)){
			$_SESSION['success'] = 'Member added successfully';
		}
		else{
			$_SESSION['error'] = 'Something went wrong while adding';
		}
	}
	else{
		$_SESSION['error'] = 'Fill up add form first';
	}

	header('location: index.php');
?>