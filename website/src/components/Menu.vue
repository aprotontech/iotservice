<template>
    <div class="side-bar">
        <el-menu router :default-openeds="open_list">
            <template v-for="(menu, index) in subMenu">
                <template v-if="menu.url !== ''">
                    <el-menu-item v-if="menu.attrs.param === 0" :index="`${index}`" :route="{ name: menu.url, params: {idx: menu.attrs.param}}" :key="menu.url" >
                        <i :class="menu.attrs.icon"></i>
                        {{ menu.title }}
                    </el-menu-item>
                    <el-menu-item v-else :index="`${index}`" :route="{ name: menu.url}" :key="menu.url">
                        <i :class="menu.attrs.icon"></i>
                        {{ menu.title }}
                    </el-menu-item>
                </template>
                <el-submenu v-else :index="`${index}`">
                    <template slot="title"><i :class="menu.attrs.icon"></i>{{ menu.title }}</template>
                    <el-menu-item-group>
                        <div v-for="(subitem, idx) in menu.children">
                            <el-menu-item  :index="`${index}-${idx}`" :route="{ name: subitem.url}" :key="subitem.url">
                                {{ subitem.title }}
                            </el-menu-item>
                        </div>
                    </el-menu-item-group>
                </el-submenu>
            </template>
        </el-menu>

    </div>
</template>
<script>
    import Commons from '@/config/commons';
    import Bus from '@/utils/bus';

    export default {
        name: 'main-menu',
        data() {
            return {
                open_list: ["0"],
                currentAppName: '应用名称',
                mainMenuId: Bus.mainMenuId,
                subMenu: []
            }
        },
        watch: {
            mainMenuId: function () {
                this.loadMenu(this.mainMenuId)
            }
        },
        mounted() {
            this.loadMenu(this.mainMenuId)
            var myself = this
            Bus.$on('menu-update', function(menuId) {
                myself.mainMenuId = menuId
            })
        },
        methods: {
            loadMenu(menuId) {
                if (!menuId) return;
                var localMenu = sessionStorage.getItem('localMenu');
                if (localMenu && (typeof localMenu) == 'string') {
                    var localMenu = JSON.parse(localMenu)
                    for (var i in localMenu) {
                        if (localMenu[i].attrs && localMenu[i].attrs.mainMenuId == menuId) {
			                console.log('load submenu from sessionStorage')
                            this.subMenu = localMenu[i].children
                            return;
                        }
                    }
                }

                this.$http.post(this.$API.ROLE_MEUN, {
                    mainMenuId: menuId,
                    system: 'aproton'
                })
                    .then((resp) => {
                        let m = resp.data
                        if(m.rc == 0) {
                            this.subMenu = m.menus
                        }
                    })
                    .catch((error) => {
                        Commons.handleApiError(this, error);
                    });
            }
        }
    }

</script>
<style lang="scss" scoped>
    .side-bar {
        min-height: 50vw;
        background-color: #eef1f6;
    }
    .apps-list{
    max-height: 80vh;
    overflow: hidden;
    overflow-y: auto;
}
</style>
