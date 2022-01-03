--TEST--
test1() Basic test
--EXTENSIONS--
electron
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension electron is loaded and working!
NULL
