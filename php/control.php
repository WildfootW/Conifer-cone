<?php
if(isset($_POST['update_status']))
{
    $write_text = $_POST['new_value'];
    $edit_file = fopen("./status.html", 'w');
    if($edit_file === false)
    {
        echo "FAILLLLED";
    }
    fwrite($edit_file, $write_text[0] . "\n", 2);
    fclose($edit_file);
    echo " SUCCESS";
}
else
{
    echo "FAILED";
}
?>
<body>
<form method="post" action="control.php" id="update_form">
    <input type="text" name="new_value">
    <input type="submit" value="Update status" name="update_status">
</form>
</body>

