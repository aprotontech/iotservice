<template>
  <div class="mode-box user-info">
    <div class="mode-box-hd">
      <h2>
        <i class="icon-aiUser"></i>
        个人信息
      </h2>
    </div>
    <div class="main-box">
      <el-form :inline="true" label-width="125px">
        <el-form-item label='邮箱:' label-width="120px">
          <el-input v-model='userInfo.email' style="width: 300px" :readonly="true" :disabled="true"></el-input>
        </el-form-item>
        <br />
        <el-form-item label='昵称：' label-width="120px">
          <el-input v-model='userInfo.name' style="width: 300px" :readonly="inforeadonly" :disabled="inforeadonly">
          </el-input>
        </el-form-item>
        <br />
        <el-form-item label='新密码：' label-width="120px" v-show="showModify">
          <el-input v-model='newPassword' placeholder="输入新密码,不填写代表不修改" autocomplete="off" style="width: 300px">
          </el-input>
        </el-form-item>
        <br />
        <el-form-item label=" " label-width="120px" v-show="showModify">
          <el-button type="primary" @click="saveInfo" :disabled="!userInfo.name">保存</el-button>
        </el-form-item>
        <el-form-item label=" " label-width="120px" v-show="showModify">
          <el-button type="cancel" @click="toModify">取消</el-button>
        </el-form-item>
        <el-form-item label=" " label-width="120px" v-show="inforeadonly">
          <el-button type="primary" @click="toModify">修改</el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>
<script>
import { upDownState, ApiMessage, formatDate } from '@/utils/util'
const dictionary = {
  custom: {
    nickname: {
      required: '昵称不能为空',
    },
  }
};
export default {
  name: 'userinfo',
  data: function () {
    return {
      newPassword: '',
      userInfo: {
        id: 0,
        email: '',
        name: '',
      },
      inforeadonly: true,
      showModify: false
    }
  },
  mounted() {
    this.getUserInfo();
  },
  methods: {
    getUserInfo: function () {
      this.$axios.get(this.$API.CURRENT_USER_INFO, {})
        .then((response) => {
          let res = response.data;
          if (res.error === "success" || res.rc == 0) {
            this.userInfo = res
          } else {
            ApiMessage(this, m.err, 'error')
          }
        }).cache((error) => {
          ApiMessage(this, error.toString(), 'error')
        })
    },
    saveInfo: function () {
      if (!this.showModify) return;
      var userPrams = this.userInfo
      if (this.showModify && this.newPassword) {
        userPrams.newPassword = this.newPassword
      }
      this.$axios.post(this.$API.CURRENT_USER_EDIT, userPrams)
        .then((response) => {
          let res = response.data;
          if (res.rc == 0) {
            this.newPassword = ''
            ApiMessage(this, '修改成功', 'success');
            this.toModify()
          } else {
            ApiMessage(this, res.err, 'error')
          }
        }).catch((error) => {
          ApiMessage(this, error.toString(), 'error')
        })
    },
    toModify: function () {
      this.inforeadonly = !this.inforeadonly
      this.showModify = !this.showModify
    }
  }
}
</script>
<style lang="less" scoped>
.user-info {
  padding: 10px 0;
}
</style>
