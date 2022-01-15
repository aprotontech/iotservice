--TEST--
electron_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('electron')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = electron_test1();

var_dump($ret);
?>
--EXPECT--
The extension electron is loaded and working!
NULL
