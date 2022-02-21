# 时间同步

## 接口定义

+ 接口URL： /api/time
+ 接口方法： POST
+ 请求参数

| 请求参数 | 类型 | 可选 | 意义 |
| - | - | - | - |
| microSecond | bool | 可选 |  是否同时返回毫秒时间（毫秒时间作为小数部分），默认为false |


+ 返回值

| 返回值 | 类型 | 意义 |
| - | - | - |
| rc | string | 错误码 |
| now | float | 当前时间戳（UNIX时间戳），单位为秒 |

+ 错误码

| 错误码 | 意义 |
| - | - |
| 0 | 成功 |

## 接口示例

+ 请求参数示例
``` json
{"microSecond":true}
```

+ 返回示例
``` json
{
    "rc": "0",
    "now": 1645456753.422354
}
```

## CURL测试
``` bash
curl -v -XPOST -d '{"microSecond":true}' http://api.aproton.tech/api/time
```