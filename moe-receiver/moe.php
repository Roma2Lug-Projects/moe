<?php

ini_set('display_errors', 'On');
error_reporting(E_ALL | E_STRICT);

$db = new SQLite3('/home/roma2lug/moe-receiver/users.sqlite', SQLITE3_OPEN_READONLY);
#$db = sqlite:/var/www/database/mac.db;
#sqlite::memory;

$results = $db->query('SELECT Name, LastIn FROM Users WHERE LastIn!=\'0\' GROUP BY Name HAVING max(LastIn) ORDER BY Name;');
#echo "Aperto db";
$array = array();
$i = 0;
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {
	#echo $row['Name']." | ".$row['LastIn']."\n";
	$timeact =  time();
	if($timeact<=$row['LastIn']){
		$array[$i] = $row['Name'];
		$i += 1;
	}
}

$json_string = json_encode($array, JSON_UNESCAPED_SLASHES);

echo $json_string;

?>
