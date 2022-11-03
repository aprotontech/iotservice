<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonRuntimeErrorHandlerTest extends ProtonTestCase
{

    public function testCodeError()
    {
        $count = 0;
        Proton\Runtime::setErrorHandler(function ($coroutine, $error) use (&$count) {
            utlog("Coroutinue[$coroutine]");
            utlog("Error=" . var_export($error, true));
            $count = 1;
            Proton\Runtime::stop();
        });

        Proton\go(function () {
            $obj = null;
            $x = $obj->name;
            echo $x;
        });

        Proton\Runtime::start();

        $this->assertEquals(1, $count);
    }
}
