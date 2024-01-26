<?php
	session_start();
	include_once('connection.php');

	if(isset($_POST['edit'])){
		$id = $_POST['id'];
		$full_name = $_POST['full_name'];
		$birthday = $_POST['birthday'];
		$apartment_number = $_POST['apartment_number'];
		$UID = $_POST['UID'];
		$license_plate = $_POST['license_plate'];
		$email = $_POST['email'];
		$phone_number = $_POST['phone_number'];
		$expiration_date = $_POST['expiration_date'];
		$sql = "UPDATE users SET full_name = '$full_name', birthday = '$birthday', apartment_number = '$apartment_number', UID = '$UID', license_plate = '$license_plate', email = '$email', phone_number = '$phone_number', expiration_date = '$expiration_date' WHERE id = '$id'";

		//use for MySQLi OOP
		if($conn->query($sql)){
			$_SESSION['success'] = 'Member updated successfully';
		}
		
		else{
			$_SESSION['error'] = 'Something went wrong in updating member';
		}
	}
	else{
		$_SESSION['error'] = 'Select member to edit first';
	}

	header('location: index.php');

?>