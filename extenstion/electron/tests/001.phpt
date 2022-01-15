--TEST--
Check if electron is loaded
--SKIPIF--
<?php
if (!extension_loaded('electron')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "electron" is available';
?>
--EXPECT--
The extension "electron" is available
