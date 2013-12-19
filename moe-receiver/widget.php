< ? php ini_set('display_errors', 'On');
error_reporting(E_ALL | E_STRICT);

$json = file_get_contents("http://160.80.101.36/moe.php");
$counter = 0;

$arr = json_decode($json, true);
foreach($arr as $item)
{
	echo '<font color="green">'.$item.'</font><br>';
	//echo "<br>";
	$counter = $counter + 1;
}

if ($counter == 0) {
	echo 'Nessuno disponibile.<br>';
	echo '<a href="http://lug.uniroma2.it/prenotazione-assistenza/">Prenotati!</a><br>';
}

? >
