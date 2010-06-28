<?php
define(PP_MAX, 12);
define(PP_MIN, 2);

if(!isset($_GET["pp_size"]))
	$pp_size = 8;
else
	$pp_size = $_GET["pp_size"];
if($pp_size < PP_MIN)
	$pp_size = PP_MIN;
elseif($pp_size > PP_MAX)
	$pp_size = PP_MAX;
?>
<script>
function pp_update()
{
	var i,j;
	var pp = "";

	for(i=<?php echo $pp_size;?>-1;i>=0;i--) {
		for(j=0;j<<?php echo $pp_size;?>;j++) {
			if(document.getElementById('pp'+i+'_'+j).checked)
				pp += '1';
			else
				pp += '0';	
		} 
		pp += ';';
	} 
	document.getElementById("pp").value = pp;
}

function pp_reload()
{
	window.location.href = '<?php echo $_SERVER["PHP_SELF"];?>'
		+ '?pp_size='
		+ document.getElementById('pp_size').value;
}
</script>
<body>
<form>
Grid Size: 
<select id="pp_size" onChange="pp_reload();">
<?php
for($i=PP_MIN;$i<=PP_MAX;$i++) {
	?>
	<option <?php if($i == $pp_size) echo "selected";?>>
	<?php echo $i;?>
	</option>
	<?php
}
?>
</select>

<table border=1 cellspacing=0 cellpadding=0>
<?php
for($i=0;$i<$pp_size;$i++) {
	?>
	<tr>
	<?php
	for($j=0;$j<$pp_size;$j++) {
		?>
		<td>
		<input type=checkbox id="pp<?php echo $i."_".$j;?>"
			onChange="pp_update();">
		</td>
		<?php
	}
	?>
	</tr>
	<?php
}
?>
</table>

<br>
g_partyPanzersPattern <input type=text size=100 id=pp readonly=true>
</form>
<br>
<a href="pp.phps">source</a>
</body>
