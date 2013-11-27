<?php

$db = new SQLite3('/home/roma2lug/moe/mac.db', SQLITE3_OPEN_READONLY);

$results = $db->query('SELECT name, isin FROM mac ORDER BY isin DESC, name');

$i = 0;
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {	
	$array[$i] = $row;
	$i += 1;
}

$json_string = json_encode($array, JSON_UNESCAPED_SLASHES);

echo $json_string;

?>
