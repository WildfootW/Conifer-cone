<?php
if(isset($_POST['update_status']))
{
    $write_text = $_POST['new_value'];
    $edit_file = fopen("./status.html", 'w');
    fwrite($edit_file, $write_text);
    fclose($edit_file);
}
?>
<body>
<form method="post" action="control.php" id="update_form">
    <input type="text" name="new_value">
    <input type="submit" value="Update status" name="update_status">
</form>
</body>

