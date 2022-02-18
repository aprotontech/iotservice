<template>
    <div class="box-401">
        <h1>您没有浏览这个页面的权限</h1>
        <ul>
            <li>
                <Button size="large" type="text" @click="backHome">返回首页</Button>
                <Button size="large" type="text">返回上一页({{ second }}s)</Button>
            </li>
        </ul>
    </div>
</template>
<script>
export default {
    name: 'NotFound',
    data: function () {
        return {
            second: 5,
            timer: null
        }
    },
    mounted() {
        this.timer = setInterval(() => {
            if (this.second === 0) this.backPrev()
            else this.second--
        }, 1000)
    },
    methods: {
        backHome() {
            this.$router.replace({
                name: 'home'
            })
        },
        backPrev() {
            this.$router.go(-1)
        }
    },
    beforeDestroy() {
        clearInterval(this.timer)
    }
}
</script>
<style scoped>
.box-401 {
    box-sizing: border-box;
    margin: 0 auto;
    width: 100%;
    background-color: #f7f8fa;
    height: 100%;
    height: 100vh;
    padding: 150px 30px;
    font-size: 16px;
    text-align: center;
    ul {
        width: 100%;
        padding: 20px 0;
        li {
            margin-right: 10px;
            display: inline-block;
        }
        a {
            color: #1aad19;
        }
    }
    h1 {
        color: #d0d1d3;
        font-size: 20px;
        font-weight: 600;
    }
}
</style>
