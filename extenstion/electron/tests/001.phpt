--TEST--
Check if electron is loaded
--EXTENSIONS--
electron
--FILE--
<?php
echo 'The extension "electron" is available';
?>
--EXPECT--
The extension "electron" is available
