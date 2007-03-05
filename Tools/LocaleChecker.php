<?php
$SolutionDir = "../";
$LocaleDir = "Release/locale/";
$MasterLocale = "nl";
$OutputFile = "./locale-report.html";
ob_start();
?>

<html>
  <head>
    <title>Locale Checker</title>
    <style type="text/css">
      BODY,TH,TD {
        font-family:Verdana,arial,sans-serif;
        font-size:10px;
      }
      
      TR.good TD {
        background-color:#DDFFDD;
      }
      
      TR.incomplete TD {
        background-color:#FFAAAA;
      }
      
      TR:hover TD {
        background-color:Yellow;
      }
    </style>
  </head>
  
  <body>
    <strong>Solution dir:</strong> <?php echo $SolutionDir ?><br/>
    <strong>Master locale:</strong> <?php echo $MasterLocale ?><br/>
    <strong>Locale dir:</strong> <?php echo $LocaleDir ?><br/>
    <strong>Time generated:</strong> <?php echo date("d-M-y H:m"); ?><br/>
  
    <?php
    $DirLocale = opendir($SolutionDir.$LocaleDir);
    $Locales = array();
    $LocaleMap = array(); // nl => array("key"=>"value") 
    $Doubles = array();
    
    function ParseLocaleFile($fn) {
      global $Doubles;
      
      $lines = file($fn);
      $map = array();
      foreach($lines as $line) {
        if(strlen(trim($line))>0) {
          $data = explode(":",$line);
          if(isset($map[$data[0]])) {
            $Doubles[] = $data[0];
          }
          $map[$data[0]] = $data[1];
        }
      }
      
      return $map;
    }

    // Discover locale files
    while (($file = readdir($DirLocale)) !== false) {
      if($file{0}!="." && is_dir($SolutionDir.$LocaleDir.$file)) {
        $Locales[] = $file;
        $files = opendir($SolutionDir.$LocaleDir."/".$file);
        $filelist = array();
        while(($lfile=readdir($files))!==false) {
          $abs = $SolutionDir.$LocaleDir."/".$file."/".$lfile;
          if(!is_dir($abs) && $lfile{0}!='.') {
            $filelist[] = $abs;
            if(!isset($LocaleMap[$file])) {
              $LocaleMap[$file] = array();
            }
            $LocaleMap[$file] = array_merge($LocaleMap[$file],ParseLocaleFile($abs));
          }
        }
      }
    }

    echo "<table border='1'>";
    echo "<tr><th>Key</th>";
    foreach($LocaleMap as $locale=>$v) {
      echo "<th>".$locale."</th>";
    }
    echo "</tr>";
    $missing = 0;
    //print_r($LocaleMap);
    foreach($LocaleMap[$MasterLocale] as $key=>$value) {      
      $allSet = true;
      foreach($LocaleMap as $locale=>$v) {
        if(!isset($LocaleMap[$locale][$key])) {
          $allSet = false;
          break;
        }
      }
      
      echo "<tr class=".($allSet?"good":"incomplete").">";
      echo "<td>$key</td>";
      
      foreach($LocaleMap as $locale=>$v) {
        if(isset($LocaleMap[$locale][$key])) {
          $localizedValue = $LocaleMap[$locale][$key];
          echo "<td><abbr title=\"".htmlentities(addslashes($localizedValue))."\">".substr($localizedValue,0,25)."</abbr></td>";
        }
        else {
          $missing++;
          echo "<td></td>";
        }
      }
      
      echo "</tr>";
    }
    echo "</table>";
    ?>
    Keys in master locale: <?php $total = count($LocaleMap[$MasterLocale]); echo $total; ?>, total <?php echo $missing ?> missing keys, <span style='font-size:50px;'><?php echo round(100-100*($missing/$total),4) ?>%</span> complete.
    <br/>
    Keys doubly defined in some locales: <?php implode(", ",$Doubles); ?>.
  </body>
</html>
<?php
$data = ob_get_flush();
$fp = fopen($OutputFile, "w");
fwrite($fp, $data);
fclose($fp);
?>
