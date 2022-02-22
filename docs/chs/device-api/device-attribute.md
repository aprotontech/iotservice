# 设备属性上报

## 接口定义

+ 接口URL： /api/device/attrs
+ 接口方法： POST
+ 请求参数

| 请求参数 | 类型 | 可选 | 意义 |
| - | - | - | - |
| clientId | string | 必填 |  设备唯一ID |
| timestamp | int | 可选 |  设备上报时的时间戳（UNIX时间戳），单位为秒或者毫秒都可以 |
| attrs | object | 必填 |  设备上报的属性值（只需要上报更新的值，不更新的值可以不上报） |
| attrs.{key} | bool/int/string/float | 必填 | 设备任意属性key（可参考示例） |


+ 返回值

| 返回值 | 类型 | 意义 |
| - | - | - |
| rc | int | 错误码 |
| timestamp | int | 当前时间戳（UNIX时间戳），单位为毫秒 |

+ 错误码

| 错误码 | 意义 |
| - | - |
| 0 | 成功 |

## 接口示例

+ 请求参数示例+
``` json
{
    "clientId": "F1010F1F000003",
    "attrs" : {
        "testInt": 10,
        "testBool": true,
        "testDouble":10.2,
        "testString":"xyz"
    },
    "timestamp": 2645546226000
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
curl -v -XPOST -d '{"clientId":"F1010F2F000000","attrs":{"localIp":"192.168.3.31"},"timestamp":2645546226000}' http://api.aproton.tech/api/device/attrs
```