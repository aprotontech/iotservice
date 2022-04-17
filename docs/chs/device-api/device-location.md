# 设备位置上报

## 接口定义

+ 接口URL： /api/location/report
+ 接口方法： POST
+ 请求参数

| 请求参数 | 类型 | 可选 | 意义 |
| - | - | - | - |
| clientId | string | 必填 |  设备唯一ID |
| showDetail | bool | 可选 | 是否返回详细地址，默认位true，如果为false(返回值中的address将不会显示) |
| wifiList | object | 必填 |  设备上报的附近WIFI列表（服务器会调用第三方API通过WIFI列表定位） |
| wifiList.mac | string | 必填 | WIFI的mac地址 |
| wifiList.signal | string | 必填 | WIFI的信号强度signal |
| wifiList.ssid | string | 必填 | WIFI的SSID（WIFI名） |


+ 返回值

| 返回值 | 类型 | 意义 |
| - | - | - |
| rc | int | 错误码 |
| location | object | 定位结果 |
| location.latitude | double | 纬度 |
| location.longitude | double | 精度 |
| location.country | string | 国家 |
| location.province | string | 省 |
| location.city | string | 城市 |
| location.address | string | 详细地址（showDetail=true时返回） |



+ 错误码

| 错误码 | 意义 |
| - | - |
| 0 | 成功 |

## 接口示例

+ 请求参数示例+
``` json
{
    "clientId": "F1010F1F000003",
    "wifiList":[
        {
            "mac":"11111",
            "signal":10,
            "ssid":"roobo"
        }
    ]
}
```

+ 返回示例
``` json
{
    "rc": 0,
    "now": 2645456753422
}
```

## CURL测试
``` bash
curl -v -XPOST -d '{"clientId":"F1010F2F000000","attrs":{"localIp":"192.168.3.31"},"timestamp":2645546226000}' http://api.aproton.tech/api/location/report
```