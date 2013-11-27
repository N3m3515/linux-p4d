<?php

include("pChart/class/pData.class.php");
include("pChart/class/pDraw.class.php");
include("pChart/class/pImage.class.php");

include("config.php");
include("functions.php");

  // -------------------------
  // establish db connection

  mysql_connect($mysqlhost, $mysqluser, $mysqlpass);
  mysql_select_db($mysqldb);
  mysql_query("set names 'utf8'");
  mysql_query("SET lc_time_names = 'de_DE'");

  // -------------------------
  // get last time stamp

  $strQuery = "select max(time), DATE_FORMAT(max(time),'%d. %M %Y   %H:%i') as maxPretty from samples;";
  $result = mysql_query($strQuery);
  $row = mysql_fetch_array($result, MYSQL_ASSOC);
  $max = $row['max(time)'];
  $maxPretty = $row['maxPretty'];

  // ----------------
  // init

  $day   = isset($_GET['sday'])   ? $_GET['sday']   : (int)date("d");
  $month = isset($_GET['smonth']) ? $_GET['smonth'] : (int)date("m");
  $year  = isset($_GET['syear'])  ? $_GET['syear']  : (int)date("Y");
  $range = isset($_GET['range'])  ? $_GET['range']  : 1;

  // ----------------
  // Status

  $strQuery = "select * from samples where time = '" . $max . "' and address = 1 and type = 'UD';";
  $result = mysql_query($strQuery);
  $row = mysql_fetch_array($result, MYSQL_ASSOC);

  echo "Status:<br>\n";
  echo $row['text'] . "\n";
  echo $row['text'] . "\n";

  echo "<br><br><br><br>\n";

  // ----------------
  // 

  echo "<form name='navigation' method='get'>\n";

  echo datePicker("Start", "s", $year, $day, $month);

  echo "Bereich: ";
  echo "   <select name=\"range\">\n";
  echo "      <option value='1' "  . ($range == 1  ? "SELECTED" : "") . ">Tag</option>\n";
  echo "      <option value='7' "  . ($range == 7  ? "SELECTED" : "") . ">Woche</option>\n";
  echo "      <option value='31' " . ($range == 31 ? "SELECTED" : "") . ">Monat</option>\n";
  echo "   </select>\n";

  echo "   <input type=submit value=\"Go\">";

  echo "</form>\n";

  $from = date_create_from_format('!Y-m-d', $year.'-'.$month.'-'.$day)->getTimestamp();

  // ------------------
  // tabelle

  echo "  <table width=\"70%\" border=1 cellspacing=0 rules=rows style=\"position:absolute; top:70px; left:50px\">\n";

  echo "    <tr style=\"color:white\" bgcolor=\"#000099\"><td/><td/><td><center>" . $maxPretty . "</center><td/><td/></tr>\n";

  echo "      <tr style=\"color:white\" bgcolor=\"#000099\">\n";
  echo "        <td>Id</td>\n";
  echo "        <td>Sensor</td>\n";
  echo "        <td>Type</td>\n";
  echo "        <td>Wert</td>\n";
  echo "        <td>Unit</td>\n";
  echo "      </tr>\n";

  $strQuery = sprintf("select s.address as s_address, s.type as s_type, s.time as s_time, s.value as s_value, f.title as f_title, f.unit as f_unit 
              from samples s, valuefacts f where f.state = 'A' and f.address = s.address and f.type = s.type and s.time = '%s';", $max);

  $result = mysql_query($strQuery);

  $i = 0;

  while ($row = mysql_fetch_array($result, MYSQL_ASSOC))
  {
     $value = $row['s_value'];
     $title = $row['f_title'];
     $unit = $row['f_unit'];
     $address = $row['s_address'];
     $type = $row['s_type'];

     $url = "<a href=\"#\" onclick=\"window.open('detail.php?width=1200&height=600&address=$address&type=$type&from=" . $from . "&range=" . $range . " ','_blank'," 
        . "'scrollbars=yes,width=1200,height=600,resizable=yes,left=120,top=120')\">";
     
     if ($i++ % 2)
        echo "   <tr bgcolor=\"#83AFFF\">\n";
     
     echo "      <td>" . $address . "</td>\n";   
     echo "      <td>" . $type . "</td>\n";   
     echo "      <td>" . $url . $title . "</a></td>\n";
     echo "      <td>$value</td>\n";
     echo "      <td>$unit</td>\n";
     echo "   </tr>\n";
  }

  echo "  </table><br>\n";

  mysql_close();

echo "</html>\n";
?>