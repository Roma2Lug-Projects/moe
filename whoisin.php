<?php

//Legenda 
echo '<b>' ."Legenda:". '</b>';
echo '<font color="green"><b>' ." Presente ". '</b></font>';
echo '<font color="red"><b>' ."Assente". '</b></font><br><br>';

//json opening
$json = file_get_contents("http://xxx.xxx.xxx.xxx/moe.php");

//json decoding
$arr = json_decode($json,true);
//
foreach($arr as $item) {
             if($item['isin']==1){
                      echo '<font color="green">' .$item['name']. '</font><br>';  
             }
             else{
                      echo '<font color="red">' .$item['name']. '</font><br>';
             }
}

?>
