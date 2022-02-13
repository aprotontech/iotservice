# Proton-IotService

Proton-IotService 是一款开源的Iot平台，集成了设备管理后台，设备接入SDK。是一套完整的物联网解决方案，可用于快速构建物联网相关业务系统。

# 快速部署
如果想要快速体验项目，可以直接使用docker的一键化部署能力，参考  [部署手册](./INSTALL.md)。

当然你也可以从头开始编译项目并部署，参考 [编译手册](./BUILD.md)

# 核心能力

## 设备管理后台
设备管理后台API接口基于Laravel开发，前端采用VUE开发。

## 设备SDK
平台也提供了一套用于快速接入设备的SDK，采用C开发。

# 架构设计


# 目录结构

```
+
|--- backend           # 管理台API以及设备API
|--+ extenstion        
|  |-- eletron         # PHP扩展
|--- website           # 管理台前端页面
|--- build             # 编译相关代码
|--- deploy            # 部署相关代码
```