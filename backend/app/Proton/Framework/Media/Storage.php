<?php

namespace Proton\Framework\Media;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Storage as LaravelStorage;

class Storage
{
    protected static $url;

    public static function saveFile(Request $request, $topic, $formFile)
    {
        if (!$request->hasFile($formFile)) {
            return rc_error(404, "not found input form file($formFile)");
        }

        $file = $request->file($formFile);
        if (!$file->isValid()) {
            return rc_error(404, "input form file($formFile) invalidate");
        }

        $day = date("Ymd");
        $path = $file->store("$topic/$day");

        return rc_success([
            'url' => self::getUrl($path),
            'file' => $path
        ]);
    }

    public static function saveLocalFile($orgFilePath, $topic)
    {
        if (!file_exists($orgFilePath)) {
            return rc_error("404", "not found local file($orgFilePath)");
        }
        $day = date("Ymd");
        $path = LaravelStorage::copy($orgFilePath, "$topic/$day");
        return rc_success([
            'url' => self::getUrl($path),
            'file' => $path
        ]);
    }

    public static function saveFileContent($content, $topic, $fileExt)
    {
        $day = date("Ymd");
        $title = md5(microtime(true) . $topic . getmypid() . rand());
        $path = "$topic/$day/$title.$fileExt";
        if (!LaravelStorage::put($path, $content)) {
            return rc_error("501", "save file to ($path) failed");
        }
        return rc_success([
            'url' => self::getUrl($path),
            'file' => $path
        ]);
    }

    protected static function getUrl($path)
    {
        if (!self::$url) {
            self::$url = env('MEDIA_URL_PREFIX', '');
        }
        return self::$url . '/' . $path;
    }
};
