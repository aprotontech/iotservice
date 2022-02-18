<template>

  <div class="media-list">
    <div class="breadcrumb-bar">
      <el-breadcrumb separator="/">
        <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
        <el-breadcrumb-item :to="{ name:'sntasklist' }">任务列表</el-breadcrumb-item>
        <el-breadcrumb-item><span>新建任务</span></el-breadcrumb-item>
      </el-breadcrumb>
    </div>
    <div class="main-box">
      <el-form :inline="true" label-width="125px">

        <el-form-item label="应用" label-width="120px">
          <el-select v-model="task.app_id" placeholder="" style="width: 300px" clearable filterable>
             <el-option v-for="item in appList" :key="item.appid" :label="item.name+' ('+item.appid+')'" :value="item.appid"></el-option>
          </el-select>
        </el-form-item>
        <br/>
        <el-form-item label='任务名:' label-width="120px">
          <el-input v-model='task.name' style="width: 300px"></el-input>
        </el-form-item>
        <br/>
        <el-form-item label='SN前缀:' label-width="120px">
          <el-input v-model='task.prefix' style="width: 300px"></el-input>
        </el-form-item>
        <br/>
        <el-form-item label='SN开始:' label-width="120px">
          <el-input v-model='task.start' style="width: 300px"></el-input>
        </el-form-item>
        <br/>

        <el-form-item label='数量:' label-width="120px">
          <el-input style="width: 300px" v-model="task.count" type="number"></el-input>
        </el-form-item>
        <br/>
        <el-form-item label="方式:" label-width="120px">
          <el-select style="width: 300px" class="item-choose" v-model="task.type" size="small">
            <el-option
              v-for="(item,index) in modeOptions"
              :key="index"
              :label="item.label"
              :value="item.value"
            ></el-option>
          </el-select>
        </el-form-item>
        <br/>
        <el-form-item label='描述：' label-width="120px">
          <el-input v-model='task.comment' style="width: 300px" type="textarea"></el-input>
        </el-form-item>
        <br/>

        <el-form-item label=" " label-width="120px">
          <el-button type="primary" @click="doInsert" :disabled="disabledBtnQuery" :loading="showLoading">确定</el-button>
        </el-form-item>
      </el-form>

      <br/>
    </div>
  </div>


</template>
<script>
  import {upDownState, ApiMessage, formatDate} from '@/utils/util'

  export default {
    name: 'createsntask',
    data() {
      return {
        task: {
          name: '',
          prefix: '',
          count: 0,
          type: '0',
          start: '',
          app_id: '',
          comment: '',
        },
        appList: [],
        modeOptions: [{
          value: '0',
          label: '在线生成'
        }, {
          value: '1',
          label: '预置设备'
        }
        ],
        disabledBtnQuery: false,
        showLoading: false,
      }
    },
    props: {},
    created() {
    },
    computed: {},
    filters: {},
    mounted() {
      this.loadApplist()
    },
    methods: {
      loadApplist() {
          this.$http.get(this.$API.APP_LIST, {params: {permission:'sn_task'}})
              .then((resp) => {
                  const data = resp.body;
                  console.log(data)
                  this.appList = data.appList;
              })
              .catch((error) => {
                  ApiMessage(this, error.toString(), 'error')
              });
      },
      doInsert() {
        this.task.count = parseInt(this.task.count)
        this.task.type = parseInt(this.task.type)
        this.$http.post(this.$API.DEVICE_SNTASK_NEW, this.task)
          .then((response) => {
            let m = response.data
            if (m.rc == 0) {
              ApiMessage(this, '添加成功', 'success');

              this.$router.push({
                name: 'sntasklist',
              });
            } else {
              ApiMessage(this, m.err, 'error')
            }
          })
          .catch((error) => {
            ApiMessage(this, error.toString(), 'error')
          });
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

