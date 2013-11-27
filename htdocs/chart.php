<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html>
  <head>
    <title>main</title>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
    <style type="text/css">
      #aSelect { position:static; width:500px; background-color:#83AFFF; border:1px solid #804000; padding:10px; border-radius:10px; }
    </style>
  </head>

  <body>
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

  // ----------------
  // init

  $day   = isset($_GET['sday'])   ? $_GET['sday']   : (int)date("d");
  $month = isset($_GET['smonth']) ? $_GET['smonth'] : (int)date("m");
  $year  = isset($_GET['syear'])  ? $_GET['syear']  : (int)date("Y");
  $range = isset($_GET['range'])  ? $_GET['range']  : 1;

  echo "<br>\n";
  echo " <div id=\"aSelect\">";
  echo "  <form name='navigation' method='get'>\n";
  echo "Zeitraum der Charts: <br>\n";
  echo datePicker("Start", "s", $year, $day, $month);

  echo "   <select name=\"range\">\n";
  echo "      <option value='1' "  . ($range == 1  ? "SELECTED" : "") . ">Tag</option>\n";
  echo "      <option value='7' "  . ($range == 7  ? "SELECTED" : "") . ">Woche</option>\n";
  echo "      <option value='31' " . ($range == 31 ? "SELECTED" : "") . ">Monat</option>\n";
  echo "   </select>\n";

  echo "   <input type=submit value=\"Go\">";

  echo "  </form>\n";
  echo " </div>";

  $from = date_create_from_format('!Y-m-d', $year.'-'.$month.'-'.$day)->getTimestamp();

  echo "<br>\n";
  $condition = "address in (" . $addrs_char1 . ")";
  echo "<img src='detail.php?width=1000&height=500&from=" . $from . "&range=" . $range . "&condition=" . $condition . "'>\n";
  echo "<br><br>\n";

  $condition = "address in (" . $addrs_char2. ")";
  echo "<img src='detail.php?width=1000&height=500&from=" . $from . "&range=" . $range . "&condition=" . $condition . "'>\n";
  echo "<br><br>\n";
?>
