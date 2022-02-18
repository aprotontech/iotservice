<template>
<header id="header">
    <div class="inner-header clearfix">
        <el-row :gutter="10">
            <el-col :xs="24" :sm="4" :md="4" :lg="4">
                <h1 class="logo">
                    <a :href="this.$API.ROOT_URL">
                        APROTON
                    </a>
                </h1>
            </el-col>
            <el-col :xs="24" :sm="20" :md="20" :lg="20">
                <div class="user-box">
                    <a @click="showUserInfo">{{userName}}</a>
                    <span>|</span>
                    <a class="linka" @click="userLogout">退出</a>
                </div>

                <nav class='nav'>
                    <ul>
                       <template v-for="(menu, index) in roleMenu">
                          <li>
                              <a :href='menu.url'>{{menu.title}}</a>
                          </li>
                       </template>
                    </ul>
                </nav>
            </el-col>
        </el-row>
    </div>
</header>
</template>

<script>
import Commons from '@/config/commons';
import VueCookie from 'vue-cookie'

export default {
    name: 'header-bar',
    data() {
            return {
                userName: '',
                platformList: [],
                roleMenu: [],
            }
    },
    computed: {
        
    },
    mounted() {
        this.loadCurrentUser();
        this.updateMenu();
    },
    methods: {
        handleCommand(cmd) {
              let url = '';
              for(let i in this.platformList) {
                 if(cmd == this.platformList[i].code) {
                    url = this.platformList[i].url;
                    break;
                 }
              }
              if(cmd == 'DeviceService') {
                this.$router.push({name: 'welcome'});
              } else {
                if(url != '') {
                    window.open(url);
                } else {
                    this.$message({
                        message: '尚未开通,尽情期待!',
                        type: 'warning'
                    });
                }
              }
              console.log(url);
            },
            loadCurrentUser() {
                const userName = sessionStorage.getItem('userName');
                console.log('name='+userName);
                if (userName) {
                    this.userName = userName;
                } else {
                    let url = '/web/passport/session';
                    this.$http.get(url).then(function (response) {
                        let m = response.body;
                        if (m.error == 'success' || m.rc == '0') {
                            sessionStorage.setItem('userName', m.name)
                            sessionStorage.setItem('userAccount', m.email)

                            this.userName = m.name
                        } else {
                            this.$router.push('/login');
                        }
                    });
                }
            },
            showUserInfo() {
                this.$router.push({
                  name: 'userinfo',
                })
            },
            handleCommandMenu(cmd) {
                console.log(cmd);
                if (cmd.url !== 'empty') {
                    if (cmd.type !== 'bot') {
                        window.location.href = cmd.url
                    } else {
                        window.open(cmd.url)
                    }
                }
            },
            userLogout() {
                this.$http.post(this.$API.USER_LOGOUT, {})
                    .then((resp) => {
                        let res = resp.data;
                        console.log(res)
                        if (res.error == 'success' || res.rc == 0) {
                            this.$message({
                                message: '登出成功，跳转到登录页中。。。',
                                type: 'info'
                            });
                        }
                        this.toLogin();
                    })
                    .catch((error) => {
                        Commons.handleApiError(this, error);
                        this.toLogin();
                    });
            },
            toLogin() {
             
                sessionStorage.removeItem('localMenu')
                sessionStorage.removeItem('userName')
                sessionStorage.removeItem('userAccount')
             
	            this.$router.push('/login')
            },
            updateMenu() {
                var localMenu = sessionStorage.getItem('localMenu');
                if (localMenu && (typeof localMenu) == 'string') {
                    console.log('load menu from sessionStorage')
                    this.roleMenu = JSON.parse(localMenu);
                    return;
                }
                this.$http.post(this.$API.ROLE_MEUN, {
                    mainMenuId: 'root',
                    system: 'aproton'
                })
                    .then((resp) => {
                        let m = resp.data
                        if(m.rc == 0) {
                            this.roleMenu = m.menus
                            sessionStorage.setItem('localMenu', JSON.stringify(m.menus))
                        }
                    })
                    .catch((error) => {
                        Commons.handleApiError(this, error);
                    });
            }
        }

    }
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style lang="scss" scoped>
#header {
    min-height: 60px;
    border-bottom: 1px solid #e6e6e6;
}

.logo {
    background-color: #303030;
    font-weight: normal;
    font-size: 22px;
    a {
        display: block;
        text-align: center;
        height: 60px;
        line-height: 60px;
        overflow: hidden;
        color: #fff;

        img {
            width: 90%;
        }
    }
}

.nav {
    float: left;
    line-height: 60px;
    height: 100%;
    font-size: 14px;
    ul {
        width: 100%;
        li {
            float: left;
            padding: 0 30px;
            font-size: 14px;
            a{
                display: block;
                &.router-link-active{
                    color: #00a3ff;
                }
            }
            &.item-dropdown {
                line-height: 40px;
                padding: 10px 30px;
                .el-dropdown-link {
                    cursor: pointer;
                }
            }
        }
    }
}

.user-box {
    padding: 0 3%;
    font-size: 14px;
    float: right;
    line-height: 60px;
    height: 100%;
        span {
            margin: 0px 5px;
        }
    }

    .linka {
        cursor: pointer;
    }
</style>
