<?php
	function generateRow(){
		$contents = '';
		include_once('connection.php');
		$sql = "SELECT * FROM users";

		//use for MySQLi OOP
		$query = $conn->query($sql);
		while($row = $query->fetch_assoc()){
			$contents .= "
			<tr>
				<td>".$row['id']."</td>
				<td>".$row['full_name']."</td>
				<td>".$row['birthday']."</td>
				<td>".$row['apartment_number']."</td>
				<td>".$row['UID']."</td>
				<td>".$row['license_plate']."</td>
				<td>".$row['email']."</td>
				<td>".$row['phone_number']."</td>
				<td>".$row['expiration_date']."</td>
			</tr>
			";
		}
		return $contents;
	}

	require_once('tcpdf/tcpdf.php');  
    $pdf = new TCPDF('P', PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);  
    $pdf->SetCreator(PDF_CREATOR);  
    $pdf->SetTitle("Print");  
    $pdf->SetHeaderData('', '', PDF_HEADER_TITLE, PDF_HEADER_STRING);  
    $pdf->setHeaderFont(Array(PDF_FONT_NAME_MAIN, '', PDF_FONT_SIZE_MAIN));  
    $pdf->setFooterFont(Array(PDF_FONT_NAME_DATA, '', PDF_FONT_SIZE_DATA));  
    $pdf->SetDefaultMonospacedFont('helvetica');  
    $pdf->SetFooterMargin(PDF_MARGIN_FOOTER);  
    $pdf->SetMargins(PDF_MARGIN_LEFT, '10', PDF_MARGIN_RIGHT);  
    $pdf->setPrintHeader(false);  
    $pdf->setPrintFooter(false);  
    $pdf->SetAutoPageBreak(TRUE, 10);  
    $pdf->SetFont('helvetica', '', 11);  
    $pdf->AddPage();  
    $content = '';  
    $content .= '
      	<h2 align="center">Print</h2>
      	<h4>Members Table</h4>
      	<table border="1" cellspacing="0" cellpadding="3">  
           <tr>  
                <th width="3%">ID</th>
				<th width="20%">Fullname</th>
				<th width="15%">Birthday</th>
				<th width="20%">Apartment number</th> 
				<th width="20%">UID</th>
				<th width="10%">License plate</th>
				<th width="10%">Email</th>
				<th width="10%">Phone number</th> 
				<th width="15%">Expiration date</th> 
           </tr>  
      ';  
    $content .= generateRow();  
    $content .= '</table>';  
    $pdf->writeHTML($content);  
    $pdf->Output('users.pdf', 'I');
	

?>