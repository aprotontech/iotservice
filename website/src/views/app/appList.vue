<template>
  <div class="mode-box fullscreen-box">
    <el-form :inline="true" :model='filter'>
      <el-form-item label="">
        <el-input v-model="filter.query" placeholder="appId"></el-input>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" icon="search" @click="getAppList" :disabled="disabledBtnQuery"
          :loading="disabledBtnQuery">查询</el-button>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" @click="jumpToCreateApp">创建</el-button>
      </el-form-item>
    </el-form>
    <el-table ref="multipleTable" :data="appList" stripe border tooltip-effect="dark" style="width: 100%">
      <el-table-column prop="appId" label="appId">
      </el-table-column>
      <el-table-column prop="name" label="应用名">
      </el-table-column>
      <el-table-column prop="creator" label="创建人">
      </el-table-column>
      <el-table-column prop="created_at" label="创建时间">
      </el-table-column>
    </el-table>
    <div class="pagination-box">
      <el-pagination @size-change="handleSizeChange" @current-change="handleCurrentChange"
        :current-page.sync="filter.page" :page-sizes="[20, 50, 100]" :page-size="filter.pageSize"
        layout="total, sizes, prev, pager, next, jumper" :total="pageTotal">
      </el-pagination>
    </div>
  </div>
</template>
<script type="text/babel">
import { upDownState, ApiMessage, formatDate } from '@/utils/util'
export default {
  name: 'applist',
  data: function () {
    return {
      disabledBtnQuery: false,
      pageTotal: 0,
      filter: {
        query: '',
        page: 1,
        pageSize: 20
      },
      appList: [],
    }
  },
  components: {},
  created() {
    this.getAppList()
  },
  mounted() {
  },
  methods: {
    getAppList() {
      let appParam = this.filter;
      appParam['permissions'] = ['admin_manager']
      this.disabledBtnQuery = true
      this.$axios.post(this.$API.APPICATION_APPLIST, appParam)
        .then(res => {
          this.disabledBtnQuery = false
          if (res.data.rc == 0) {
            this.appList = res.data.apps
            this.pageTotal = res.data.total
          } else {
            ApiMessage(this, res.data.err, 'error')
          }
        }).catch(error => {
          this.disabledBtnQuery = false
          ApiMessage(this, error.toString(), 'error')
        })
    },
    jumpToCreateApp() {
      this.$router.push({
        name: 'createapp',
      })
    },
    handleSizeChange(val) {
      this.filter.pageSize = val
      this.getAppList()
    },
    handleCurrentChange(val) {
      this.filter.page = val
      this.getAppList()
    }
  }
}

</script>
<style lang="less">
.app-List {
  display: flex;
  flex-wrap: wrap;
  margin: 0 -5px;

  li {
    width: 16.666%;
    padding: 0px 5px 15px;

    .app-icon {
      margin: 0 auto;
      text-align: center;
      color: #999;
      height: 50px;
      width: 50px;
      border-radius: 50px;
      line-height: 50px;
      background-color: #f4f4f4;
    }

    h4 {
      text-align: center;
      font-weight: normal;
      margin: 5px 0 10px;
      font-size: 16px;
    }

    p {
      text-align: center;
      font-size: 14px;
      color: #666;
      height: 63px;
      overflow: hidden;
      display: -webkit-box;
      -webkit-line-clamp: 3;
      /*! autoprefixer: off */
      -webkit-box-orient: vertical;
      /* autoprefixer: on */
    }

    a {
      padding: 20px;
      display: block;
      height: 200px;
      overflow: hidden;
      border: 1px solid #eee;
      background-color: #fff;
      color: #303133;
      transition: 0.3s;

      &:hover,
      &:focus {
        box-shadow: 0 15px 30px rgba(0, 0, 0, 0.1);
      }
    }
  }
}

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
