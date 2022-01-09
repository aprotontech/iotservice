<template>
<div class="edit-container">
    <div class="breadcrumb-bar">
        <el-breadcrumb separator="/">
            <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
            <el-breadcrumb-item>序列号导出</el-breadcrumb-item>
        </el-breadcrumb>
    </div>
    <div class="main-box">
      <el-form :inline="true" :model="formFuzzy">
        <el-row type="flex" class="row-bg">
          <el-col :span="24">
            <el-form-item label="应用" label-width="120px">
              <el-select v-model="formFuzzy.app_id" placeholder="" clearable filterable @change='getSnRange' style="width: 300px">
                <el-option v-for="item in appList" :key="item.appid" :label="item.name+' ('+item.appid+')'" :value="item.appid"></el-option>
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
        <el-row type="flex" class="row-bg">
          <el-col :span="24">
            <el-form-item label="起始序列号" label-width="120px">
             <el-input v-model="formFuzzy.startId" placeholder="输入开始序列号" @keyup.enter.native='onFuzzySubmit' style="width: 300px"></el-input>
            </el-form-item>
          </el-col>
        </el-row>
        <el-row type="flex" class="row-bg">
          <el-col :span="24">
            <el-form-item label="结束序列号" label-width="120px">
             <el-input v-model="formFuzzy.endId" placeholder="输入结束序列号" @keyup.enter.native='onFuzzySubmit' style="width: 300px"></el-input>
            </el-form-item>
          </el-col>
        </el-row>
        <el-row type="flex" class="row-bg">
          <el-col :span="24">
            <el-form-item label=" " label-width="120px">
              <el-button type="primary" icon="primary" @click="onFuzzySubmit" :disabled="disabledBtnQuery"
                         :loading="showLoading">导出
              </el-button>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
   </div>
   <div>
     <p v-show='canDownload'>点击 <a :href="downurl" target="_blank">这里</a> 下载文件</p>
   </div>
</div>
</template>

<script>
import { ApiMessage, formatDate } from '@/utils/util'

export default {
    name: 'deviceexport',
    data() {
        return {
            appList: [],
            formFuzzy: {
                startId: '',
                endId: '',
                app_id: '',
            },
            disabledBtnQuery: false,
            showLoading: false,
            downurl: '',
            canDownload: false
        }
    },
    created() {
    },
    computed: {
    },
    mounted() {
        this.loadApplist()
    },
    watch: {
        '$route': 'fetchDetail'
    },
    filters: {
        formatDate2(d) {
            return formatDate(d,'yyyy-MM-dd hh:mm:ss');
        }
    },
    methods: {
        loadApplist() {
            this.$http.get(this.$API.APP_LIST, {params: {permission: 'device_export'}})
                .then((resp) => {
                    const data = resp.body;
                    console.log(data)
                    this.appList = data.appList;
                })
                .catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                });
        },
        getSnRange() {
            this.$http.post(this.$API.SN_RANGE, {app_id: this.formFuzzy.app_id})
                .then((resp) => {
                    const data = resp.body;
                    console.log(data)
                    this.formFuzzy.startId = data.minSn;
                    this.formFuzzy.endId = data.maxSn;
                })
                .catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                });
        },
        onFuzzySubmit() {
             this.disabledBtnQuery = true
             this.canDownload = false
             this.showLoading = true
             this.$http.post(this.$API.SN_DOWN_URL, this.formFuzzy)
               .then((response) => {
                 this.disabledBtnQuery = false
                 this.showLoading = false

                 let m = response.data
                 console.log(m);
                 if (m.rc == 0) {
                     this.downurl = m.downloadUrl;
                     this.canDownload = true
                     //window.location = this.downurl;
                     return true;
                 } else {
                   ApiMessage(this, m.err, 'error')
                 }
               })
               .catch((error) => {
                 this.showLoading = false
                 this.disabledBtnQuery = false
                 ApiMessage(this, error.toString(), 'error')
               });

        }
    }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style lang="scss" scoped>
</style>
