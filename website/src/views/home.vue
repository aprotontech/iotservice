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
                        <router-view></router-view>
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
    export default {
        name: 'home',
        data() {
            return {
                home: 'aproton home'
            }
        },
        mounted() {
        },
        watch: {
        },
        methods: {
        }
    }

</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style lang="scss" scoped>

</style>
