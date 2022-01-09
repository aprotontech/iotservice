<template>
  <div class="edit-container">
    <div class="breadcrumb-bar">
      <el-breadcrumb separator="/">
        <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
        <el-breadcrumb-item :to="{ name: 'devicelist' }">设备列表</el-breadcrumb-item>
        <el-breadcrumb-item>{{ this.$route.query.clientId }}</el-breadcrumb-item>
      </el-breadcrumb>
    </div>
    <div class="main-box">

      <div class="mode-box">
        <h3 class="mode-title">基本信息 </h3>
        <div class="mode-content">
          <el-table
            :data="deviceInfo"
            style="width: 100%">
            <el-table-column
              prop="client_id"
              label="序列号"
              width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="uuid"
              label="UUID"
              align="center">
            </el-table-column>
            <el-table-column
              prop="activeTime"
              label="激活时间"
              align="center">
            </el-table-column>
            <el-table-column
              prop="disableTime"
              label="禁用时间"
              align="center">
            </el-table-column>

          </el-table>

        </div>


      </div>

      <div class="mode-box">
        <h3 class="mode-title">连接状态</h3>
        <div class="mode-content">

          <el-table
            :data="deviceOnline"
            style="width: 100%">
            <el-table-column
              prop="isOnline"
              label="在线状态"
              width="200" align="center">
            </el-table-column>
            <el-table-column
              prop="onlineTime"
              label="最近在线时间"
              width="300" align="center">
            </el-table-column>

            <el-table-column
              prop="offlineTime"
              label="最近离线时间"
              align="center">
            </el-table-column>

          </el-table>

        </div>
      </div>

      <div class="mode-box">
        <h3 class="mode-title">位置信息 </h3>
        <div class="mode-content">
          <el-table
            :data="deviceLocation"
            border
            style="width: 100%">
            <el-table-column
              prop="ip"
              label="IP"
              width="200" align="center">
            </el-table-column>
            <el-table-column
              prop="longitude"
              label="经度"
              width="120" align="center">
            </el-table-column>
            <el-table-column
              prop="latitude"
              label="纬度"
              width="120" align="center">
            </el-table-column>

            <el-table-column
              prop="updateTime"
              label="更新时间"
              width="180"
              align="center">
            </el-table-column>
            <el-table-column
              prop="address"
              label="详细地址"
              align="center">
            </el-table-column>
          </el-table>

        </div>


      </div>

      <div class="mode-box">
        <h3 class="mode-title">认证信息 </h3>
        <div class="mode-content" style="white-space:pre-wrap">
          <el-table
            :data="deviceSecret"
            border
            style="width: 100%">
            <el-table-column
              prop="pub_key"
              label="公钥"
              align="center">
              <template slot-scope="scope">
                <p v-for="item in scope.row.pub_key">
                  {{ item }}
                </p>
              </template>
            </el-table-column>
            <el-table-column
              prop="token"
              label="会话"
              align="center">
            </el-table-column>
          </el-table>
        </div>
      </div>

      <div class="mode-box">
        <h3 class="mode-title">版本信息 </h3>
        <div class="mode-content">
          <el-table
            :data="versionData"
            border
            style="width: 100%">
            <el-table-column
              prop="scope"
              label="类型"
              width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="name"
              label="升级包"
              width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="vname"
              label="版本名"
              width="180" align="center">
            </el-table-column>

            <el-table-column prop="created_at" label="更新时间" show-overflow-tooltip align="center" min-width="140">
            </el-table-column>

            <el-table-column prop="updated_at" label="最近上报时间" show-overflow-tooltip align="center" min-width="140">
            </el-table-column>

          </el-table>

        </div>
      </div>

      <div class="mode-box">
        <h3 class="mode-title">设备属性 </h3>
        <div class="mode-content">
          <el-table
            :data="deviceAttrs"
            border
            style="width: 100%">
            <el-table-column
              prop="property"
              label="ID"
              width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="name"
              label="名称"
              min-width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="device_val"
              label="值"
              min-width="180" align="center">
            </el-table-column>
            <el-table-column
              prop="report_time"
              label="修改时间"
              min-width="180" align="center">
            </el-table-column>

          </el-table>
        </div>
      </div>

      <!--div class="mode-box">
        <h3 class="mode-title">绑定信息&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<el-button type="primary"
                                                                                        v-if="cleanBindVisible"
                                                                                        @click="cleanUpBind()">
          清除绑定关系
        </el-button>
        </h3>

        <div class="mode-content">
          <el-table ref="table"
                    :data="groupData"
                    border
                    style="width: 100%">
            <el-table-column
              prop="phone"
              label="手机号"
              align="center">
            </el-table-column>
            <el-table-column
              prop="role"
              label="身份信息"
              align="center">
            </el-table-column>

            <el-table-column prop="bindTime" label="绑定时间" show-overflow-tooltip align="center" min-width="140">
              <template slot-scope="scope">
                <span v-if="scope.row.bindTime > 0">{{ scope.row.bindTime | formatDate2 }}</span>
                <span v-else></span>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </div-->

      <div class="mode-box">
        <div class="mode-content center">
        </div>
      </div>

    </div>
  </div>
</template>

<script>
  import {ApiMessage, formatDate, randomStr} from '@/utils/util'

  var base64 = require('base-64');

  export default {
    name: 'devicedetail',
    data() {
      return {
        deviceOnline: [
          {
            isOnline: '',
            onlineTime: '',
            offlineTime: ''
          }
        ],
        deviceInfo: [{
          client_id: '',
          token: '',
          activeTime: '',
          disableTime: ''
        }],
        deviceLocation: [{
          address: '-',
          ip: '-',
          longitude: '-',
          latitude: '-',
          updateTime: '-',
        }],
        deviceAttrs: [],
        deviceSecret: [{
          token: '-',
          pub_key: ['-'],
        }],
        versionData: [],
        groupData: [],
        cleanBindVisible: false
      }
    },
    created() {
      this.fetchDetail()
      this.fetchDeviceVersionInfo();
    },
    computed: {},
    mounted() {
    },
    watch: {
    },
    filters: {
      formatDate2(d) {
        return formatDate(d, 'yyyy-MM-dd hh:mm:ss');
      }
    },
    methods: {
      fetchDetail() {
        let params = {
          app_id: this.$route.query.appId,
          client_id: this.$route.query.clientId
        }
        this.$http.post(this.$API.DEVICE_DETAIL, params)
          .then((response) => {
            let m = response.data
            if (m.rc == 0) {
              this.deviceInfo = [{
                client_id: m.basic.client_id,
                uuid: m.basic.uuid,
                activeTime: m.basic.active_time ? m.basic.active_time  : '未激活',
                disableTime: m.basic.deleted_at ? m.basic.deleted_at : '正常',
                token: ''
              }]

              this.deviceOnline = [{
                isOnline: m.basic.is_online ? '在线' : '离线',
                onlineTime: m.basic.online_time ? m.basic.online_time : '-',
                offlineTime: m.basic.offline_time ? m.basic.offline_time : '-'
              }]

              if (m.location) {
                this.deviceLocation = [{
                  address: m.location.address,
                  longitude: m.location.longitude,
                  latitude: m.location.latitude,
                  city: m.location.city,
                  updateTime: m.location.updated_at ? m.location.updated_at : '-',
                  ip: m.location.ip
                }]
              }
              if (m.secret) {
                this.deviceSecret = [{
                  token: '',
                  pub_key: m.secret.public_key.split('\n')
                }]
              }
              if (m.attrs) {
                  this.deviceAttrs = m.attrs
              }
            }
          })
          .catch((error) => {
            ApiMessage(this, error.toString(), 'error')
          });

      },
      fetchDeviceVersionInfo() {
        return;
        let params = {
          app_id: this.$route.query.appId,
          client_id: this.$route.query.clientId
        }
        this.$http.post(this.$API.OTA_GET_VERSION, params)
          .then((response) => {
            let m = response.data
            if (m.rc == 0) {
              this.versionData = m.versions;
            }
          })
          .catch((error) => {
            ApiMessage(this, error.toString(), 'error')
          });
      },


    }
  }
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style lang="scss" scoped>
  .edit-box {
    margin-top: 20px;
    border-top: 1px solid #efefef;
    padding: 20px 0;
    max-width: 900px;

    .el-form-item {
      width: 48%;
      &.form-item-lg {
        width: 100%;
        audio,
        .el-button {
          vertical-align: middle;
          margin-right: 10px;
        }
      }
    }

  }

  .mode-box {
    .mode-title {
      font-size: 18px;
      font-weight: normal;
      margin-bottom: 20px;
      padding-left: 10px;
      line-height: 20px;
      border-left: 4px solid #00a3ff;
    }

  }

  .mode-content {
    margin-bottom: 30px;
    dl {
      float: left;
      display: block;
      width: 100%;
      font-size: 14px;
      line-height: 32px;
      padding: 5px 0;
      &.mode-item {
        width: 50%;
      }
    }
    dt {
      float: left;
    }
    dd {
      overflow: hidden;
      zoom: 1;
    }
  }

  .mode-btn {
    padding: 10px 0;
    text-align: center;
  }

</style>
