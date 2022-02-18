<style lang="less" scoped>
    .layout{
        border: 1px solid #d7dde4;
        background: #f5f7f9;
        position: relative;
        border-radius: 4px;
        overflow: hidden;
    }
    .layout-header-bar{
        background: #fff;
        box-shadow: 0 1px 1px rgba(0,0,0,.1);
         .header-r {
            float: right;
            height: auto;
            padding-right: 20px;
            line-height: 64px;
        }
    }
    .layout-logo-left{
        width: 90%;
        height: 30px;
        background: #5b6270;
        border-radius: 3px;
        margin: 15px auto;
    }
    .layout-logo {
        width: 100%;
        margin: 15px auto 30px;
        color: #fff;
        text-align: center;
        h2 {
            a {
                display: block;
                color: #fff;
                height: 35px;
                margin: 0 20px;
            }
            .login-icon {
                float: left;
                background-color: #1aa1e9;
                font-size: 24px;
                height: 35px;
                line-height: 35px;
                width: 35px;
                text-align: center;
                border-top-right-radius: 10px;
                border-bottom-right-radius: 10px;
                margin-right: 10px;
                font-style: normal;
            }
            p {
                overflow: hidden;
                vertical-align: middle;
                transition: all 0.2s ease 0.2s;
                text-align: left;
                max-width: 120px;
                span {
                    font-size: 14px;
                    display: block;
                    width: 100%;
                    font-weight: normal;
                }
                b {
                    display: block;
                    width: 100%;
                    font-weight: normal;
                    font-size: 12px;
                    line-height: 1.2;
                    font-family: Arial, Helvetica, sans-serif;
                }
            }
        }
    }
    .menu-icon{
        transition: all .3s;
    }
    .rotate-icon{
        transform: rotate(-90deg);
    }
    .menu-item span{
        display: inline-block;
        overflow: hidden;
        width: 69px;
        text-overflow: ellipsis;
        white-space: nowrap;
        vertical-align: bottom;
        transition: width .2s ease .2s;
    }
    .menu-item i{
        transform: translateX(0px);
        transition: font-size .2s ease, transform .2s ease;
        vertical-align: middle;
        font-size: 16px;
    }
    .collapsed-menu span{
        width: 0px;
        transition: width .2s ease;
    }
    .collapsed-menu i{
        transform: translateX(5px);
        transition: font-size .2s ease .2s, transform .2s ease .2s;
        vertical-align: middle;
        font-size: 22px;
    }
    .user {
        &-avator-dropdown {
            cursor: pointer;
            display: inline-block;
            vertical-align: middle;
        }
    }
    .layout-footer-center{
       text-align: center;
    }
    .ivu-breadcrumb {
        margin: 16px;
    }
    .form-search {
        position: relative;
        .ivu-form {
            padding-right: 10%;
        }
        .opt-button {
            position: absolute;
            right: 0;
            top: 0;
        }
    }
</style>
<template>
    <div class="layout">
        <Layout>

             

            <Sider ref="side1" hide-trigger collapsible :collapsed-width="78" v-model="isCollapsed">


                 <div class="layout-logo">
                        <h2>
                            <a
                                href="/"
                                title="Proton Admin"
                            >
                                <i class="login-icon">P</i>
                                <p v-show="!isCollapsed">
                                    <span>Proton Admin</span>
                                    <b>管理系统</b>
                                </p>
                            </a>
                        </h2>
                    </div>


                <Menu :active-name="activeName" theme="dark" width="auto" :class="menuitemClasses">
                     <MenuItem name="dashboard" to="/dashboard">
                        <Icon type="md-apps"></Icon>
                        <span>Dashboard</span>
                    </MenuItem>
                    <MenuItem name="accountlist" to='/accountlist'>
                        <Icon type="md-contacts"></Icon>
                        <span>用户列表</span>
                    </MenuItem>
                    <MenuItem name="roles" to='/roles'>
                        <Icon type="md-shirt" />
                        <span>角色列表</span>
                    </MenuItem>
                    <MenuItem name="permissions" to='/permissions'>
                        <Icon type="md-log-in" />
                        <span>权限列表</span>
                    </MenuItem>
                     <MenuItem name="apis" to='/apis'>
                        <Icon type="md-browsers" />
                        <span>API列表</span>
                    </MenuItem>
                     <MenuItem name="systems" to='/systems'>
                        <Icon type="md-cog" />
                        <span>系统列表</span>
                    </MenuItem>
                     <MenuItem name="menus" to='/menus'>
                        <Icon type="md-menu" />
                        <span>菜单列表</span>
                    </MenuItem>
                   
                </Menu>
            </Sider>
            <Layout>
                <Header :style="{padding: 0}" class="layout-header-bar">
                    <Icon @click.native="collapsedSider" :class="rotateIcon" :style="{margin: '0 20px'}" type="md-menu" size="24"></Icon>

                    <div class="header-r">
                     <div class="user-avator-dropdown">
                        <Dropdown @on-click="logoutHandle">
                            <Avatar
                                style="background-color:#1aa1e9"
                                icon="ios-person"
                            ></Avatar>
                            <Icon
                                :size="18"
                                type="md-arrow-dropdown"
                            ></Icon>
                            <DropdownMenu slot="list">
                                <DropdownItem>{{this.$store.state.username}}</DropdownItem>
                                <DropdownItem
                                    name="logout"
                                    divided
                                >退出登录</DropdownItem>
                            </DropdownMenu>
                        </Dropdown>
                    </div>
                    </div>

                </Header>
                <Content :style="{margin: '20px', background: '#fff', minHeight: '600px', maxHeight: '800px'}">
                    <router-view></router-view>
                </Content>
            </Layout>
        </Layout>
            <Footer class="layout-footer-center">2021- &copy; APROTON </Footer>
    
    </div>
</template>
<script>
import VueCookie from 'vue-cookie';
    export default {
         props: {
            },
        data () {
            return {
                isCollapsed: false,
            }
        },
        computed: {
            activeName() {
                return this.$route.meta.activeMenu;
            },
            rotateIcon () {
                return [
                    'menu-icon',
                    this.isCollapsed ? 'rotate-icon' : ''
                ];
            },
            menuitemClasses () {
                return [
                    'menu-item',
                    this.isCollapsed ? 'collapsed-menu' : ''
                ]
            }
        },
        methods: {
            collapsedSider () {
                this.$refs.side1.toggleCollapse();
            },
            logoutHandle(val) {
                 console.log(val);
                if(val == 'logout') {
                    let params = {
                        sessionId:VueCookie.get('WEBUI_SESSION_ID')
                    }
                    this.$axios
                         .post('/logout', params)
                         .then(response => {
                            console.log(response);
                            VueCookie.delete('WEBUI_SESSION_ID');
                            sessionStorage.removeItem('localMenu')
                            sessionStorage.removeItem('userName')
                            sessionStorage.removeItem('userAccount')
                            this.$router.push('/login');
                        });
                 }
            }
        }
    }
</script>
