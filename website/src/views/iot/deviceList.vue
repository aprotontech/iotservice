<template>

   <div class="media-list">            
        <div class="breadcrumb-bar">
            <el-breadcrumb separator="/">
                <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
                <el-breadcrumb-item>设备列表</el-breadcrumb-item>
            </el-breadcrumb>
        </div>
        <div class="main-box">
            <div>
                <el-form :inline="true" :model="formPrecise">
                    <el-form-item label="应用">
                      <el-select v-model="formPrecise.app_id" placeholder="" clearable filterable>
                         <el-option v-for="item in appList" :key="item.appid" :label="item.name+' ('+item.appid+')'" :value="item.appid"></el-option>
                      </el-select>
                    </el-form-item>
                    <el-form-item label="序列号：">
                        <el-input v-model="formPrecise.client_id" placeholder="设备唯一序列号"></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" icon="search" @click="onFuzzySubmit" :disabled="disabledBtnQuery" :loading="showLoading">查询</el-button>
                        <el-button type="text" icon="more" @click="showMore=!showMore"></el-button>
                    </el-form-item>
                 </el-form>

                 <el-form :inline="true" :model="formPrecise" v-if="showMore">
                     <el-form-item label="uuid：">
                         <el-input v-model="formPrecise.uuid"></el-input>
                     </el-form-item>
                     <el-form-item label="城市：">
                         <el-input v-model="formPrecise.city"></el-input>
                     </el-form-item>
                     <el-form-item label="最后IP：">
                         <el-input v-model="formPrecise.lastip"></el-input>
                     </el-form-item>
                   
                     <el-form-item label="激活状态：">
                         <el-select v-model="formPrecise.activeStatus" placeholder="请选择">
                             <template v-for="item in activeStatusList">
                                 <el-option :label="item.label" :value="item.id"></el-option>
                             </template>
                         </el-select>
                     </el-form-item>

                     <el-form-item label="在线状态：">
                         <el-select v-model="formPrecise.onlineStatus" placeholder="请选择">
                             <template v-for="item in onlineStatusList">
                                 <el-option :label="item.label" :value="item.id"></el-option>
                             </template>
                         </el-select>
                     </el-form-item>

                     <el-form-item label="禁用状态：">
                         <el-select v-model="formPrecise.enableStatus" placeholder="请选择">
                             <template v-for="item in enableStatusList">
                                 <el-option :label="item.label" :value="item.id"></el-option>
                             </template>
                         </el-select>
                     </el-form-item>

                     <el-form-item label="设备激活时间：">
                      <el-date-picker v-model="formPrecise.activeTime" type="datetimerange" placeholder="选择时间范围">
                          </el-date-picker>
                     </el-form-item>
                     <el-form-item label="最后上线时间：">
                         <el-date-picker v-model="formPrecise.lastOnlineTime" type="datetimerange" placeholder="选择时间范围">
                             </el-date-picker>
                     </el-form-item>

                     <el-form-item label="最后离线时间：">
                         <el-date-picker v-model="formPrecise.lastOfflineTime" type="datetimerange" placeholder="选择时间范围">
                             </el-date-picker>
                     </el-form-item>
                     

                     <el-form-item>
                         <el-button type="primary" icon="search" @click="onPreciseSubmit" :disabled="disabledBtnQuery" :loading="showLoading">高级查询</el-button>
                     </el-form-item>
                 </el-form>

                   <el-table ref="multipleTable" :data="queryData" stripe border tooltip-effect="dark" style="width: 100%"  @selection-change="handleSelectionChange">
                       <el-table-column type="selection"  width="50">
                       </el-table-column>
                       <el-table-column prop="app_id" label="app_id" width="180">
                       </el-table-column>
                       <el-table-column prop="client_id" show-overflow-tooltip label="序列号" align="center"  width="180" fixed>
                           <template slot-scope="scope">
                               <el-button type="text" @click="showDetail(scope.row.app_id,scope.row.client_id)">  {{ scope.row.client_id }}</el-button> 
                           </template>
                       </el-table-column>
                       <el-table-column prop="uuid" label="uuid" width="180">
                       </el-table-column>
                       <el-table-column prop="active_time" label="激活时间" width="180">
                            <template slot-scope="scope">
                               <span v-if="scope.row.active_time">{{scope.row.active_time}}</span>
                               <span v-else>未激活</span>
                           </template>
                       </el-table-column>
                       <el-table-column prop="is_online" label="在线状态"  show-overflow-tooltip align="center" width="100">
                           <template slot-scope="scope">
                               <span v-if="scope.row.is_online > 0" style="color:green">在线</span>
                               <span v-else>离线</span>
                           </template>
                       </el-table-column>
                       <el-table-column prop="online_time" label="最后上线时间" show-overflow-tooltip align="center" min-width="140">
                       </el-table-column>
                       <el-table-column prop="ip" label="最后IP" show-overflow-tooltip align="center" min-width="120">
                       </el-table-column>
                       <el-table-column prop="city" label="城市" show-overflow-tooltip align="center" min-width="100">
                       </el-table-column>
                       <el-table-column prop="deleted_at" label="禁用状态" show-overflow-tooltip align="center" min-width="100">
                            <template slot-scope="scope">
                               <span v-if="scope.row.deleted_at > 0" style="color:red">禁用</span>
                               <span v-else>正常</span>
                           </template>
                       </el-table-column>
                   </el-table>
                   <div class="res-opt">
                       <el-row :gutter="0">
                           <el-col :xs="24" :sm="12" :md="12" :lg="14">
                               <el-button icon="close" @click="handleDisable">禁用</el-button>
                               <el-button icon="check" @click="handleEnable">解禁</el-button>
                           </el-col>
                           
                       </el-row>
                   </div>
                   <div class="pagination-box">
                       <el-pagination
                           @size-change="handleSizeChange"
                           @current-change="handleCurrentChange"
                           :current-page.sync="queryParams.page"
                           :page-sizes="[20, 50, 100]"
                           :page-size="queryParams.pageSize"
                           layout="total, sizes, prev, pager, next, jumper"
                           :total="pageTotal">
                       </el-pagination>
                   </div>
               </div>
        </div>
    </div>

 
</template>
<script>
import { upDownState, ApiMessage, formatDate } from '@/utils/util'
var base64 = require('base-64');
export default {
    name: 'devicelist',
    data() {
        return {
            disabledBtnQuery: false,
            showLoading: false,
            showMore: false,
            flag: false,
            pageTotal: 0,
            appList: [],
            formPrecise: {
                app_id: '',
                client_id: '',
                uuid: '',
                city: '',
                lastip: '',
                activeStatus: '',
                onlineStatus: '',
                enableStatus: '',
                lastOnlineTime: '',
                lastOfflineTime: '',
                activeTime: '',
            },
            multipleSelection: [],
            queryParams: {
                page: 1,
                pageSize: 20
            },
            queryData: [],
            upDownData: [],
            sourceList: [],
            activeStatusList: [
                {"label":"全部","id":0},
                {"label":"激活","id":1},
                {"label":"未激活","id":2}
            ],
            onlineStatusList: [
                {"label":"全部","id":0},
                {"label":"在线","id":1},
                {"label":"离线","id":2}
            ],
            enableStatusList: [
                {"label":"全部","id":0},
                {"label":"启用","id":1},
                {"label":"禁用","id":2}
            ]
        }
    },
    props: {
       
    },
    created () {
        this.fetchQueryData()
        this.loadApplist()
    },
    computed: {
    },
    filters: {
        formatDate2(d) {
            return formatDate(d,'yyyy-MM-dd hh:mm:ss');
        }
    },
    mounted() {
    },
    watch: {
        '$route': 'fetchQueryData'
    },
    methods: {
        //显示详情
        showDetail(appid, sn) {
            this.$router.push({
		name: 'devicedetail', 
                query: {
                    appId: appid,
                    clientId: sn
                }
            });
        },
        // 获取查询
        fetchQueryData() {
            let params = {
                app_id : this.formPrecise.app_id,
                size: this.queryParams.pageSize,
                page: this.queryParams.page
            }

            if (this.formPrecise.client_id) {
                params.client_id = this.formPrecise.client_id
            }
            if (this.formPrecise.uuid) {
                params.uuid = this.formPrecise.uuid;
            }
            if (this.formPrecise.city) {
                params.city = this.formPrecise.city;
            }
            if (this.formPrecise.lastip) {
                params.ip = this.formPrecise.lastip;
            }
            if (this.formPrecise.activeStatus > 0) {
                params.is_active = this.formPrecise.activeStatus;
            }
            if (this.formPrecise.onlineStatus > 0) {
                params.is_online = this.formPrecise.onlineStatus;
            }
            if (this.formPrecise.enableStatus > 0) {
                params.is_deleted = this.formPrecise.enableStatus;
            }
            if (this.formPrecise.lastOnlineTime && this.formPrecise.lastOnlineTime[1]) {
                params.online_stime = formatDate(new Date(this.formPrecise.lastOnlineTime[0]),'yyyy-MM-dd hh:mm:ss');
                params.online_etime = formatDate(new Date(this.formPrecise.lastOnlineTime[1]),'yyyy-MM-dd hh:mm:ss');
            }
            if (this.formPrecise.lastOfflineTime && this.formPrecise.lastOfflineTime[1]) {
                params.offline_stime = formatDate(new Date(this.formPrecise.lastOfflineTime[0]),'yyyy-MM-dd hh:mm:ss');
                params.offline_etime = formatDate(new Date(this.formPrecise.lastOfflineTime[1]),'yyyy-MM-dd hh:mm:ss');
            }
            if (this.formPrecise.activeTime && this.formPrecise.activeTime[1]) {
                params.active_stime = formatDate(new Date(this.formPrecise.activeTime[0]),'yyyy-MM-dd hh:mm:ss');
                params.active_etime = formatDate(new Date(this.formPrecise.activeTime[1]),'yyyy-MM-dd hh:mm:ss');
            }

            this.disabledBtnQuery = true;
            this.showLoading = true;

            this.$http.post(this.$API.DEVICE_LIST, params)
                .then((response)=>{
                    let m = response.data
                    if(m.rc == 0) {
                        this.queryData = m.list;
                        this.pageTotal = m.total;
                    }
                    
                    this.disabledBtnQuery = false;
                    this.showLoading = false;
                })
                .catch((error) => {
                    this.disabledBtnQuery = false;
                    this.showLoading = false;
                    ApiMessage(this, error.toString(), 'error')
                });
        },
        loadApplist() {
            this.$http.get(this.$API.APP_LIST, {params:{permission: 'device_view'}})
                .then((resp) => {
                    const data = resp.body;
                    console.log(data)
                    this.appList = data.appList;
                })
                .catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                });
        },
       
        // 模糊查询
        onFuzzySubmit() {
            this.fetchQueryData();
        },
        // 高级查询
        onPreciseSubmit() {
            this.fetchQueryData();
        },
        querystr(obj) {
            var str = [];
            for (var p in obj) {
                if (obj.hasOwnProperty(p)) {
                    str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
                }
            }
            return str.join("&");
        },
        // 选择
        handleSelectionChange(val) {
            this.multipleSelection = val;
        },
        handleDisable() {
            let paramsData = {
                ids: []
            } 
            if(this.multipleSelection.length === 0) {
                ApiMessage(this,'请选择设备', 'warning');
                return false;
            }
            this.multipleSelection.forEach(function(val, index) {
                paramsData.ids.push(val.id);
            });
            if(confirm('确定要禁用吗?')) {
                this.$http.post(this.$API.DEVICE_DELETE, paramsData)
                    .then((response) => {
                        console.log(response);
                        if (response.data.result === 0) {
                            ApiMessage(this, '禁用成功！', 'success');
                            this.fetchQueryData();
                        } else {
                            ApiMessage(this, '禁用失败！', 'error');
                        }
                    })
                    .catch((error) => {
                        ApiMessage(this, error.toString(), 'error');
                    });
            }
        },
        handleEnable() {
            let paramsData = {
                ids: []
            } 
            if(this.multipleSelection.length === 0) {
                ApiMessage(this,'请选择设备', 'warning');
                return false;
            }
            this.multipleSelection.forEach(function(val, index) {
                paramsData.ids.push(val.id);
            });
            if(confirm('确定要启用吗?')) {
                this.$http.post(this.$API.DEVICE_ENABLE, paramsData)
                    .then((response) => {
                        console.log(response);
                        if (response.data.result === 0) {
                            ApiMessage(this, '启用成功！', 'success');
                            this.fetchQueryData();
                        } else if (response.data.result == -5015) {
                            alert('启用失败，设备数量已超过设定的最大数量！');
                        } else {
                            ApiMessage(this, '启用失败！', 'error');
                        }
                    })
                    .catch((error) => {
                        ApiMessage(this, error.toString(), 'error');
                    });
            }
        },
        handleSizeChange(val) {
            this.queryParams.pageSize = val
            this.fetchQueryData()
            console.log(`每页 ${val} 条`);
        },
        handleCurrentChange(val) {
            this.queryParams.page = val
            this.fetchQueryData()
            console.log(`当前页: ${val}`);
        }
    }
}
</script>
<style lang="scss" scoped>
.res-opt{
    padding: 20px 0;

    .btn-right{
        text-align: right;
    }
}
.showMsgBox {
    width: 600px
}
</style>
