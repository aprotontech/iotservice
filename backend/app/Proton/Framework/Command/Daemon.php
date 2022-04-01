<?php

namespace Proton\Framework\Command;

final class Signal
{
    public static $signo = 0;
    protected static $ini = null;
    public static function set($signo)
    {
        self::$signo = $signo;
    }
    public static function get()
    {
        return (self::$signo);
    }
    public static function reset()
    {
        self::$signo = 0;
    }
}

class Daemon
{
    /* config */
    const pidfile     = __CLASS__;
    const uid        = 80;
    const gid        = 80;
    const sleep    = 5;

    protected $pool     = NULL;
    protected $config    = array();

    public function __construct($uid, $gid, $class)
    {
        $this->pidfile = '/tmp/' . substr(md5(__FILE__), 8, 8) . '.pid';
        $this->uid = $uid;
        $this->gid = $gid;
        $this->class = $class;
        $this->classname = get_class($class);

        $this->signal();
    }

    public function signal()
    {
        pcntl_signal(SIGHUP,  function ($signo) /*use ()*/ {
            //echo "\n This signal is called. [$signo] \n";
            printf("The process has been reload.\n");
            Signal::set($signo);
        });
    }

    private function daemon()
    {
        if ($this->status()) {
            echo "The file $this->pidfile exists.\n";
            exit();
        }

        $pid = pcntl_fork();
        if ($pid == -1) {
            die('could not fork');
        } else if ($pid) {
            // we are the parent
            //pcntl_wait($status); //Protect against Zombie children
            exit(0);
        } else {
            file_put_contents($this->pidfile, getmypid());
            //posix_setuid(self::uid);
            //posix_setgid(self::gid);
            return (getmypid());
        }
    }


    public function start()
    {
        $this->daemon();
    }

    private function stop()
    {

        if (file_exists($this->pidfile)) {
            $pid = file_get_contents($this->pidfile);
            posix_kill($pid, 9);
            unlink($this->pidfile);
        }
    }

    private function reload()
    {
        if (file_exists($this->pidfile)) {
            $pid = file_get_contents($this->pidfile);
            //posix_kill(posix_getpid(), SIGHUP);
            posix_kill($pid, SIGHUP);
        }
    }

    private function status()
    {
        if (file_exists($this->pidfile)) {
            $pid = file_get_contents($this->pidfile);
            return system(sprintf("ps ax | grep %s | grep -v grep", $pid));
        }
    }

    private function help($proc)
    {
        printf("%s start | stop | restart | status | help \n", $proc);
    }
}
