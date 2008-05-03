<?php
require_once("../globals.php");
require_once("xpsave.php");
if(!isset($_REQUEST["xpsave_sort"]))
	$_REQUEST["xpsave_sort"] = "win";
?>
<table border = 1>
<tr>
<?php
$fields = array(
	"name" => "Name",
	"win" => "PR Win %",
	"kd" => "KR K/D",
	//"lower_rating" => "Range",
	//"player_rating" => "Player Rating",
	//"player_rating_variance" => "Player Rating Deviation",
	//"killrating" => "Kill Rating",
	//"playerrating" => "Player Rating",
	//"skill[0]" => "Battle Sense",
	//"skill[1]" => "Engineering",
	//"skill[2]" => "First Aid",
	//"skill[3]" => "Signals",
	//"skill[4]" => "Light Weapons",
	//"skill[5]" => "Heavy Weapons",
	//"skill[6]" => "Covert Ops",
	"time" => "Last Seen"
	);
?>
<TD>Rank</TD>
<?php
while(list($k,$v) = each($fields)) {
	?>
	<TD><?php
	if($_REQUEST["xpsave_sort"] != $k) {
		?>
		<a href="<?php $_SERVER["PHP_SELF"];?>?xpsave_sort=<?php
		echo urlencode($k);?>">
		<b><?php echo $v;?></b></a>
		<?php
	}
	else {
		?>
		<b><?php echo $v;?></b>
		<?php
	}
	?>
	</td>
	<?php
}
reset($fields);
?>
</td>
</tr>
<?php
$xpsaves = xpsave_readconfig($GLOBALS["XPSAVE_CFG"], $_REQUEST["xpsave_sort"]);
$rank = 0;
while(list($guid,$x) = each($xpsaves)) {
	$rank++;
	?>
    <tr>
    <td>
	<?php
	echo $rank;
	?></td>
	<?php
	while(list($k,$v) = each($fields)) {
		if($x[$k] == "") {
			switch($k) {
			default:
				$x[$k] = 0;
				break;
			}
		}
		if($k == "time") {
			$x[$k] = date("Y-m-d H:i", $x[$k]);
		} else if($k == "name") {
			$x[$k] = preg_replace("/\^(.{1})/","",$x[$k]);
		}
		$x[$k] = htmlspecialchars($x[$k]);
		?>
		<td><?php
		echo ($x[$k] == "") ? "&nbsp;" : $x[$k];
		?></td>
		<?php
	}
	reset($fields);
	?>
	</tr>
	<?php
}
?>
</table>
