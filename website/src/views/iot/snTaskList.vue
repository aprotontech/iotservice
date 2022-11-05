<template>

    <div class="media-list">
        <div class="breadcrumb-bar">
            <el-breadcrumb separator="/">
                <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
                <el-breadcrumb-item>任务列表</el-breadcrumb-item>
            </el-breadcrumb>
        </div>
        <div class="main-box">
            <div>
                <el-form :inline="true" :model="formFuzzy">
                    <el-form-item label="应用">
                        <el-select v-model="formFuzzy.app_id" placeholder="" clearable filterable>
                            <el-option v-for="item in appList" :key="item.appid"
                                :label="item.name + ' (' + item.appid + ')'" :value="item.appid"></el-option>
                        </el-select>
                    </el-form-item>
                    <el-form-item label="">
                        <el-input v-model="formFuzzy.query" placeholder="SN前缀或者任务名"></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" icon="search" @click="onFuzzySubmit" :disabled="disabledBtnQuery"
                            :loading="showLoading">查询</el-button>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="jumpToNew">新增
                        </el-button>
                    </el-form-item>
                </el-form>

                <el-table ref="multipleTable" :data="queryData" stripe border tooltip-effect="dark" style="width: 100%">
                    <el-table-column prop="app_id" label="appId">
                    </el-table-column>
                    <el-table-column prop="name" label="任务名">
                    </el-table-column>
                    <!--el-table-column prop="count" label="数量">
                     </el-table-column-->
                    <el-table-column prop="prefix" label="前缀">
                    </el-table-column>
                    <el-table-column prop="start" label="开始SN">
                    </el-table-column>
                    <el-table-column prop="end" label="结束SN">
                    </el-table-column>
                    <el-table-column prop="statusString" label="状态">
                    </el-table-column>
                    <el-table-column prop="creator" label="申请人">
                    </el-table-column>
                    <el-table-column prop="created_at" label="申请时间">
                    </el-table-column>
                    <el-table-column prop="acker" label="审核人">
                    </el-table-column>
                    <el-table-column prop="acked_at" label="审核时间">
                    </el-table-column>
                    <el-table-column label="操作" show-overflow-tooltip align="center" min-width="180">
                        <template slot-scope="scope">

                            <el-button v-if="scope.row.status == 0" type="text" @click="handleAck(scope.row.id, 1)">通过
                            </el-button>
                            <el-button v-if="scope.row.status == 0" type="text" @click="handleAck(scope.row.id, 2)">拒绝
                            </el-button>
                            <el-button v-if="scope.row.status == 5" type="text" @click="handleDownload(scope.row.id)">下载
                            </el-button>
                        </template>
                    </el-table-column>
                </el-table>
                <div class="pagination-box">
                    <el-pagination @size-change="handleSizeChange" @current-change="handleCurrentChange"
                        :current-page.sync="queryParams.page" :page-sizes="[20, 50, 100]"
                        :page-size="queryParams.pageSize" layout="total, sizes, prev, pager, next, jumper"
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
    name: 'sntasklist',
    data() {
        return {
            disabledBtnQuery: false,
            showLoading: false,
            pageTotal: 0,
            appList: [],
            formFuzzy: {
                app_id: '',
                query: ''
            },
            multipleSelection: [],
            queryParams: {
                page: 1,
                pageSize: 20
            },
            queryData: [],
        }
    },
    props: {

    },
    created() {
        this.fetchQueryData()
        this.loadApplist()
    },
    computed: {
    },
    filters: {
        formatDate2(d) {
            return formatDate(d, 'yyyy-MM-dd hh:mm:ss');
        }
    },
    mounted() {
    },
    watch: {
        '$route': 'fetchQueryData'
    },
    methods: {
        // 获取查询
        fetchQueryData() {
            let params = {
                app_id: [this.formFuzzy.app_id],
                query: this.formFuzzy.query,
                size: this.queryParams.pageSize,
                page: this.queryParams.page
            }

            this.disabledBtnQuery = true;
            this.showLoading = true;

            this.$axios.post(this.$API.DEVICE_SNTASK_LIST, params)
                .then((response) => {
                    let m = response.data
                    if (m.rc == 0) {
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

        // 模糊查询
        onFuzzySubmit() {
            this.fetchQueryData();
        },
        loadApplist() {
            this.$axios.get(this.$API.APP_LIST, { params: { permission: 'sn_task' } })
                .then((resp) => {
                    const data = resp.data;
                    console.log(data)
                    this.appList = data.appList;
                })
                .catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                });
        },
        handleAck(val, status) {
            let params = {
                task_id: val,
                status: status
            }
            this.$axios.post(this.$API.DEVICE_SNTASK_ACK, params)
                .then((response) => {
                    let m = response.data
                    if (m.rc == 0) {
                        this.fetchQueryData();
                    } else {
                        ApiMessage(this, m.err, 'error');
                    }
                })
                .catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                });
        },
        jumpToNew() {
            this.$router.push({
                name: 'createsntask',
            })
        },
        handleDownload(val) {
            ApiMessage(this, '暂未实现，请到设备导出页面下载', 'error');
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
.res-opt {
    padding: 20px 0;

    .btn-right {
        text-align: right;
    }
}

.showMsgBox {
    width: 600px
}
</style>

