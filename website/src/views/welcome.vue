<template>
    <div class="home">
        <header-bar></header-bar>
        <div class="page-container">
            <el-row :gutter="10">
                <el-col :xs="24" :sm="4" :md="4" :lg="4">
                    <main-menu></main-menu>
                </el-col>
                <el-col :xs="24" :sm="20" :md="20" :lg="20">
                    <div class="grid-content">
                        <div class="edit-container">
                            <div class="main-box edit-box">
                                <div class="mode-box">
                                    <div class="mode-content">
                                    	欢迎回来
                                    </div>
                                </div>
                                
                            </div>
                    </div>
                    </div>
                </el-col>
            </el-row>
        </div>
        <footer id="footer">
            <p>aproton</p>
        </footer>
    </div>
</template>


<script>
import { ApiMessage, formatDate } from '@/utils/util'

export default {
    name: 'welcome',
    data() {
        return {
        }
    },
    created() {
    },
    computed: {
    },
    mounted() {
    },
    watch: {
    },
    filters: {
    },
    methods: {
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

    .el-form-item{
        width: 48%;
        &.form-item-lg{
            width: 100%;
            audio,
            .el-button{
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
