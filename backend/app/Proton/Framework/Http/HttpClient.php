<?php

namespace Proton\Framework\Http;

class HttpClient
{
    const RESPONSE_RAW = 0;
    const RESPONSE_OBJECT = 1;
    const RESPONSE_ARRAY = 2;
    const RESPONSE_RC_OBJECT = 3;

    protected $url;
    protected $method;

    protected $urlPrefix;

    protected $topic;
    protected $connTimeout;
    protected $recvTimeout;
    protected $proxyServer;
    protected $header;

    protected $postFile;
    protected $retry;
    protected $retryInterval;

    protected $enableForwardIp = false;
    protected $enableReqId = false;
    protected $reqPrefix = 'rcReqId=';

    protected $responseFormat;

    public function __construct($topic, $recvTimeout, $connection_timeout = 1)
    {
        $this->topic = $topic;
        $this->recvTimeout = $recvTimeout;
        $this->connTimeout = $connection_timeout;
        $this->postFile = false;
        $this->retry = 0;
        $this->retryInterval = 0;
        $this->responseFormat = HttpClient::RESPONSE_RAW;
    }

    public function setResponseFormat($format = HttpClient::RESPONSE_RC_OBJECT)
    {
        $this->responseFormat = $format;
        return $this;
    }

    public function setRetry($retry, $retryInterval = 0.02)
    {
        $this->retry = $retry;
        $this->retryInterval = $retryInterval;
        return $this;
    }

    public function setEnableReqId($enableReqId, $reqPrefix = 'rcReqId=')
    {
        $this->enableReqId = $enableReqId;
        $this->reqPrefix = $reqPrefix;
        return $this;
    }

    public function setIsPostFile($file = true)
    {
        $this->postFile = $file;
        return $this;
    }

    public function setTopic($topic)
    {
        $this->requestTopic = $topic;
        return $this;
    }

    public function setProxyServer($proxy)
    {
        $this->proxyServer = $proxy;
        return $this;
    }

    public function setEnableForwardIp($enable)
    {
        $this->enableForwardIp = $enable;
        return $this;
    }

    public function setUrlPrefix($prefix)
    {
        $this->urlPrefix = $prefix;
        return $this;
    }

    public function setHeader($header)
    {
        $this->header = $header;
        return $this;
    }

    public function get($url, $data = '', $header = [], $timeout = 0)
    {
        return $this->runGetRequest($url, $data, $header, $timeout);
    }

    public function post($url, $data, $header = [], $timeout = 0)
    {
        return $this->runPostRequest($url, $data, $header, $timeout);
    }

    protected function runGetRequest($url, $data, $header, $timeout)
    {
        $url = $this->appendToUrl($url, $data);
        $options = $this->createUrlOptions($url, $header, $timeout);
        $options[CURLOPT_CUSTOMREQUEST] = "GET";
        return $this->do_curl_exec($options);
    }

    protected function runPostRequest($url, $data, $header, $timeout)
    {
        $data = $this->formatData($data);
        $options = $this->createUrlOptions($url, $header, $timeout);
        $options[CURLOPT_POST] = 1;
        $options[CURLOPT_FRESH_CONNECT] = 1;
        $options[CURLOPT_FORBID_REUSE] = 1;
        $options[CURLOPT_CUSTOMREQUEST] = "POST";
        $options[CURLOPT_SSL_VERIFYPEER] = false;
        $options[CURLOPT_SSL_VERIFYHOST] = false;
        $options[CURLOPT_POSTFIELDS] = $data;
        return $this->do_curl_exec($options);
    }

    protected function runSoapRequest($url, $data, $header, $timeout)
    {
        $options = $this->createUrlOptions($url, $header, $timeout);
        $options[CURLOPT_POST] = 1;
        $options[CURLOPT_FRESH_CONNECT] = 1;
        $options[CURLOPT_FORBID_REUSE] = 1;
        $options[CURLOPT_CUSTOMREQUEST] = "POST";
        $options[CURLOPT_SSL_VERIFYPEER] = false;
        $options[CURLOPT_SSL_VERIFYHOST] = false;
        $options[CURLOPT_POSTFIELDS] = $data;
        return $this->do_soap_exec($options);
    }

    protected function runDeleteRequest($url, $data, $header, $timeout)
    {
        $options = $this->createUrlOptions($url, $header, $timeout);
        $options[CURLOPT_POST] = 1;
        $options[CURLOPT_HTTP_VERSION] = CURL_HTTP_VERSION_1_1;
        $options[CURLOPT_CUSTOMREQUEST] = "DELETE";
        $options[CURLOPT_POSTFIELDS] = $this->formatData($data);
        return $this->do_curl_exec($options);
    }

    protected function runPutRequest($url, $data, $header, $timeout)
    {
        $options = $this->createUrlOptions($url, $header, $timeout);
        $options[CURLOPT_POST] = 1;
        $options[CURLOPT_HTTP_VERSION] = CURL_HTTP_VERSION_1_1;
        $options[CURLOPT_CUSTOMREQUEST] = "PUT";
        $options[CURLOPT_POSTFIELDS] = $this->formatData($data);
        return $this->do_curl_exec($options);
    }

    protected function removeRequestId($orgUrl)
    {
        $url = $orgUrl;
        $p1 = strpos($url, '?'.$this->reqPrefix);
        if ($p1 !== false) { // remove request prefix
            $p2 = strrpos($url, '&', $p1);
            if ($p2 === false) $url = substr($url, 0, $p1);
            else $url = substr($url, 0, $p1) . '?' . substr($url, $p2 + 1);

            return $url;
        }

        $p1 = strpos($url, '&'.$this->reqPrefix);
        if ($p1 !== false) {
            $p2 = strrpos($url, '&', $p1 + 1 + strlen($this->reqPrefix));
            if ($p2 === false) $url = substr($url, 0, $p1);
            else $url = substr($url, 0, $p1) . substr($url, $p2);
        }

        return $url;
    }

    protected function createUrlOptions($url, $header, $timeout)
    {
        if (!$header && $this->header) $header = $this->header;
        $this->url = $this->urlPrefix ? $this->urlPrefix.$url : $url;
        $options = array(
            CURLOPT_URL => $this->url,
            CURLOPT_HTTPHEADER => $header,
            CURLOPT_RETURNTRANSFER => 1
        );

        if ($this->proxyServer) { // proxy server
            $options[CURLOPT_PROXY] = $this->proxyServer;
        }

        $timeout = $timeout <= 0 ? $this->recvTimeout : $timeout;

        $options[CURLOPT_NOSIGNAL] = true;
        $options[CURLOPT_TIMEOUT_MS] = floor($timeout * 1000);
        $options[CURLOPT_NOSIGNAL] = true;
        $options[CURLOPT_CONNECTTIMEOUT_MS] = floor($this->connTimeout * 1000);

        return $options;
    }

    protected function appendToUrl($url, $data)
    {
        $data = $this->formatData($data);
        if ($data) {
            $pos = strpos($url , '?');
            if ($pos === false) {
                $url .= '?' . $data;
            } elseif ($pos == strlen($url) - 1) {
                $url .= $data;
            } else {
                $url .= '&' . $data;
            }
        }

        return $url;
    }

    protected function formatData($data)
    {
        if (!is_string($data) && !$this->postFile) {
            if (is_object($data) || is_array($data)) {
                $data = http_build_query($data);
            } else {
                $data = '';
            }
        }
        return $data;
    }

    private function do_soap_exec($options)
    {
        $response = $this->r_do_curl_exec($options);
        return $response;
    }

    private function do_curl_exec($options)
    {
        for ($i = 0; $i <= $this->retry; ++ $i) {
            if ($i != 0) {
                usleep($this->retryInterval * 1000000);
            }
            [$response, $error] = $this->r_do_curl_exec($options);
            if ($response || !$error) {
                break;
            }
        }

        $response = $this->formatResponse($response);

        return $response;
    }

    private function r_do_curl_exec($options)
    {
        $url = $options[CURLOPT_URL];
        $request_data = isset($options[CURLOPT_POSTFIELDS]) ? $options[CURLOPT_POSTFIELDS] : '';
        $method = $options[CURLOPT_CUSTOMREQUEST];

        $request_start_time = microtime(true);
        $ch = curl_init();
        curl_setopt_array($ch, $options);
        $response = curl_exec($ch);
        $err_no = curl_errno($ch);

        if ($err_no) {
            $error =  curl_error($ch) . ", errNo: $err_no";
        } else {
            $error = false;
            $err_no = @curl_getinfo($ch, CURLINFO_HTTP_CODE);
        }
        curl_close($ch);

        $request_usetm = floor((microtime(true) - $request_start_time) * 1000);
        $this->requestLog($request_data, $response, $request_usetm, $error, $err_no);
        return [$response, $error];
    }

    protected function requestLog($request, $response, $usedtm, $error, $code = 200)
    {
        $url = $this->removeRequestId($this->url);
        $request = is_string($request) ? $request : json_encode($request);
        $usedtm = round($usedtm, 3); // make sure time format

        $response = str_replace("\n", " ", str_replace("\r", " ", $response));
        if ($code >= 200 && $code < 300) {
            rclog_notice("[HTTP-REQUEST] [$this->topic] url($url), method($this->method), "
                ."request($request), response($response),"
                ." usedtm($usedtm), code($code), error($error)"
            );
        } else {
            rclog_error("[HTTP-REQUEST] [$this->topic] url($url), method($this->method), "
                ."request($request), response($response),"
                ." usedtm($usedtm), code($code), error($error)"
            );
        }
    }

    private function formatResponse($res)
    {
        if ($this->responseFormat == HttpClient::RESPONSE_RAW) return $res;

        switch ($this->responseFormat) {
        case HttpClient::RESPONSE_OBJECT:
            @ $m = json_decode($res, false);
            break;
        case HttpClient::RESPONSE_ARRAY:
            @ $m = json_decode($res, true);
            break;
        case HttpClient::RESPONSE_RC_OBJECT:
            @ $m = json_decode($res, true);
            if (!isset($m['rc']) || !is_string($m['rc'])) {
                $m = rc_error(500, "http request url($this->url) failed with raw response($res)");
            } else if ($m['rc'] == 0) {
                $m = rc_success($m);
            } else {
                if (isset($m['err'])) $m = rc_error($m['rc'], $m['err']);
                else $m = rc_error($m['rc'], 'unknown error');
            }
            break;
        default:
            $m = $res;
        }

        return $m;
    }

};

