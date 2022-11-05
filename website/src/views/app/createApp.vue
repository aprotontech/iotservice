<template>
  <div class="media-list">
    <div class="breadcrumb-bar">
      <el-breadcrumb separator="/">
        <el-breadcrumb-item :to="{ path: '/' }">首页</el-breadcrumb-item>
        <el-breadcrumb-item :to="{ name: 'applist' }">应用列表</el-breadcrumb-item>
        <el-breadcrumb-item><span>新建应用</span></el-breadcrumb-item>
      </el-breadcrumb>
    </div>
    <div class="main-box">
      <el-form :inline="true" label-width="125px">
        <el-form-item label='应用名:' label-width="120px">
          <el-input v-model='app.name' style="width: 300px" v-validate="'required'"></el-input>
        </el-form-item>
        <br />
        <el-form-item label='详细描述：' label-width="120px">
          <el-input v-model='app.desc' style="width: 300px" type="textarea" v-validate="'required'"></el-input>
        </el-form-item>
        <br />
        <el-form-item label=" " label-width="120px">
          <el-button type="primary" @click="onsubmit" :disabled="!app.name || !app.desc">创建</el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>
<script>
import { upDownState, ApiMessage, formatDate } from '@/utils/util'
export default {
  name: 'createapp',
  data: function () {
    return {
      app: {
        name: '',
        desc: ''
      }
    }
  },
  components: {},
  created() {
  },
  mounted() { },
  methods: {
    onsubmit() {
      this.$axios.post(this.$API.APPICATION_CREATE_APP, this.app)
        .then((res) => {
          if (res.data.rc == 0) {
            ApiMessage(this, '创建成功', 'success');

            this.$router.push({
              name: 'applist',
            });
          } else {
            ApiMessage(this, m.err, 'error')
          }
        }, function (error) {
          ApiMessage(this, error.toString(), 'error')
        })
    }
  }
}

</script>
<style lang="less">
@media screen and (min-width: 1600px) {
  .app-List {
    li {
      width: 16.666%;
    }
  }
}

@media screen and (max-width: 768px) {
  .app-List {
    li {
      width: 25%;
    }
  }
}
</style>

