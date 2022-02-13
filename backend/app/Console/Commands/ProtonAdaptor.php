<?php

namespace App\Console\Commands;

use Illuminate\Contracts\Http\Kernel;
use Illuminate\Http\Request as IlluminateRequest;
use Symfony\Component\HttpFoundation\Request as SymfonyRequest;

use Proton\HttpRequest;

class ProtonAdaptor
{
    private $kernel;
    private $illuminateRequest;
    private $illuminateResponse;

    public function __construct(Kernel $kernel)
    {
        $this->kernel = $kernel;
    }

    public function handle(HttpRequest $request)
    {
        $this->illuminateRequest = $this->protonRequestToIlluminateRequest($request);
        $this->illuminateResponse = $this->kernel->handle(
            $this->illuminateRequest
        );

        $headers = [];
        foreach ($this->illuminateResponse->headers->all() as $key => $values) {
            foreach ($values as $val) {
                array_push($headers, "$key: $val");
            }
        }

        return [
            'StatusCode' => $this->illuminateResponse->getStatusCode(),
            'Headers' => $headers,
            'Body' => $this->illuminateResponse->getContent()
        ];
    }

    public function terminate()
    {
        $this->kernel->terminate($this->illuminateRequest, $this->illuminateResponse);
    }

    private function protonRequestToIlluminateRequest($request)
    {
        $query = [];
        $srequest = [];
        $attributes = [];
        $cookies = [];
        $files = [];
        $server = [];
        $uri = $request->Path;
        $headers = [];
        $poffset = strpos($uri, '?');
        if ($poffset !== false) {
            parse_str(substr($uri, $poffset + 1), $query);

            $srequest = $query;
        }


        foreach ($request->getHeaders() as $h) {
            $kv = explode(':', $h, 2);
            if (count($kv) != 2) continue;
            $key = trim($kv[0]);
            $val = trim($kv[1]);
            $headers[$key] = $val;

            if (strtolower($key) == 'cookie') {
                foreach (explode(';', $val) as $cookie) {
                    $kv = explode('=', $cookie, 2);
                    if (count($kv) == 2) {
                        $cookies[$kv[0]] = $kv[1];
                    }
                }
            }
        }

        $remote = explode($request->getConnect()->getRemote(), ':', 2);

        $server['REQUEST_METHOD'] = $request->Method;
        $server['SERVER_NAME'] = '';
        $server['SERVER_SOFTWARE'] = 'aproton/0.1';
        $server['SERVER_PROTOCOL'] = "HTTP/1.1";
        $server['REQUEST_TIME'] = time();
        $server['REQUEST_TIME_FLOAT'] = microtime(true);
        $server['QUERY_STRING'] = $poffset !== false ? substr($uri, $poffset + 1) : '';
        $server['REMOTE_ADDR'] = count($remote) == 2 ? $remote[0] : '';
        $server['REMOTE_PORT'] = count($remote) == 2 ? $remote[1] : '';
        $server['HTTPS'] = '';
        $server['SERVER_NAME'] = '';
        $server['SERVER_NAME'] = '127.0.0.1';
        $server['SERVER_PORT'] = 80;
        $server['REQUEST_URI'] = $poffset !== false ? substr($uri, 0, $poffset) : $uri;

        foreach ($headers as $key => $val) {
            $new_key = 'HTTP_' . strtoupper(str_replace('-', '_', $key));
            $server[$new_key] = $val;
        }

        if (
            $server['REQUEST_METHOD'] == 'POST' && isset($server['HTTP_CONTENT_TYPE']) &&
            strtolower($server['HTTP_CONTENT_TYPE']) == 'application/x-www-form-urlencoded'
        ) { // append more requests
        }

        return IlluminateRequest::createFromBase(new SymfonyRequest(
            $query,
            $srequest,
            $attributes,
            $cookies,
            $files,
            $server,
            $request->getBody()
        ));
    }
}
