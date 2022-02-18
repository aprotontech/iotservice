import Vue from 'vue'
import Router from 'vue-router'

import Welcome from '@/views/welcome'
import Error from '@/views/not-found'
import Login from '@/views/user/Login.vue'
import NotFound from '@/components/NotFound'
import Error401 from '@/components/Error401'

import Home from '@/views/home'
import DeviceDetail from '@/views/iot/deviceDetail'

import AppList from '@/views/app/appList.vue'
import CreateApp from '@/views/app/createApp.vue'

import UserInfo from '@/views/user/userInfo.vue'

import Devicelist from '@/views/iot/deviceList'
import ExportDevice from '@/views/iot/deviceExport'
import SnTaskList from '@/views/iot/snTaskList'
import CreateSnTask from '@/views/iot/createSnTask'

import Dashboard from '@/views/permission/Dashboard.vue'
import AccountList from '@/views/permission/AccountList.vue'
import NewAccount from '@/views/permission/NewAccount.vue'
import RoleList from '@/views/permission/RoleList.vue'
import UserRoleList from '@/views/permission/UserRoleList.vue'
import PermissionList from '@/views/permission/PermissionList.vue'
import ApiList from '@/views/permission/ApiList.vue'
import RoleAdd from '@/views/permission/RoleAdd.vue'
import Permission from '@/views/permission/Permission.vue'
import Apiconfig from '@/views/permission/Apiconfig.vue'
import AddUserRole from '@/views/permission/AddUserRole.vue'
import GrantPermission from '@/views/permission/GrantPermission.vue'
import Systems from '@/views/permission/SystemList.vue'
import AddSystem from '@/views/permission/AddSystem.vue'
import Unauthorized from '@/views/permission/Unauthorized.vue'
import MenuList from '@/views/permission/MenuList.vue'
import Menu from '@/views/permission/Menu.vue'
import RoleMenuList from '@/views/permission/RoleMenuList.vue'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'welcome',
      meta: { requiresAuth: process.env.ENABLE_AUTH },
      component: Welcome
    },
    {
      path: '/dashboard',
      name: 'home',
      meta: { requiresAuth: process.env.ENABLE_AUTH },
      component: Welcome
    },
    {
      path: '/login',
      name: 'login',
      component: Login
    },
    {
      path: '/error',
      name: 'Error',
      meta: {
        hideInMenu: true
      },
      component: Error401
    },
    {
      path: '/error',
      name: 'Error',
      meta: { requiresAuth: process.env.ENABLE_AUTH },
      component: Error
    },
    {
      path: '/app',
      component: Home,
      meta: { requiresAuth: process.env.ENABLE_AUTH, mainMenuId: 'app' },
      children: [
        {
          path: '/',
          name: 'applist',
          component: AppList
        },
        {
          path: '/createapp',
          name: 'createapp',
          component: CreateApp
        },
      ]
    },
    {
      path: '/user',
      name: 'user',
      component: Home,
      children: [
        {
          path: '/myinfo',
          name: 'userinfo',
          component: UserInfo
        }
      ]
    },
    {
      path: '/iot',
      component: Home,
      meta: { requiresAuth: process.env.ENABLE_AUTH, mainMenuId: 'device' },
      children: [
        {
          path: 'devicelist',
          name: 'devicelist',
          component: Devicelist
        },
        {
          path: 'devicedetail',
          name: 'devicedetail',
          component: DeviceDetail
        },
        {
          path: 'deviceexport',
          name: 'deviceexport',
          component: ExportDevice
        },
        {
          path: 'sntasklist',
          name: 'sntasklist',
          component: SnTaskList
        },
        {
          path: 'createsntask',
          name: 'createsntask',
          component: CreateSnTask
        }
      ]
    },
    {
      path: '/permission',
      name: 'home',
      component: Home,
      meta: { requiresAuth: process.env.ENABLE_AUTH, mainMenuId: 'permission' },
      children: [
        {
          path: 'dashboard',
          name: 'dashboard',
          component: Dashboard,
          meta: {
            title: 'Dashboard',
            activeMenu: 'dashboard',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'accountlist',
          name: 'accountlist',
          component: AccountList,
          meta: {
            title: 'AccountList',
            activeMenu: 'accountlist',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'newaccount',
          name: 'newaccount',
          component: NewAccount,
          meta: {
            title: 'NewAccount',
            activeMenu: 'newaccount',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'roles',
          name: 'roles',
          component: RoleList,
          meta: {
            title: 'RoleList',
            activeMenu: 'roles',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'userroles/:id',
          name: 'userroles',
          component: UserRoleList,
          meta: {
            title: 'UserRoleList',
            activeMenu: 'accountlist'
          }
        }, {
          path: 'adduserrole/:id',
          name: 'adduserrole',
          component: AddUserRole,
          meta: {
            title: '',
            activeMenu: 'accountlist'
          }
        }, {
          path: 'role',
          name: 'roleadd',
          component: RoleAdd,
          meta: {
            title: 'RoleAdd',
            activeMenu: 'roles',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'role/:id',
          name: 'roleedit',
          component: RoleAdd,
          meta: {
            title: 'RoleAdd',
            activeMenu: 'roles',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'grantpermission/:id',
          name: 'grantpermission',
          component: GrantPermission,
          meta: {
            title: '',
            activeMenu: 'roles'
          }
        }, {
          path: 'permissions',
          name: 'permissions',
          component: PermissionList,
          meta: {
            title: 'PermissionList',
            activeMenu: 'permissions',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'permission',
          name: 'permissionadd',
          component: Permission,
          meta: {
            title: 'Permission',
            activeMenu: 'permissions',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'permission/:id',
          name: 'permissionedit',
          component: Permission,
          meta: {
            title: 'Permission',
            activeMenu: 'permissions',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'apis',
          name: 'apis',
          component: ApiList,
          meta: {
            title: 'ApiList',
            activeMenu: 'apis',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'api',
          name: 'apiadd',
          component: Apiconfig,
          meta: {
            title: 'Apiconfig',
            activeMenu: 'apis',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'api/:id',
          name: 'apiedit',
          component: Apiconfig,
          meta: {
            title: 'Apiconfig',
            activeMenu: 'apis',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'systems',
          name: 'systems',
          component: Systems,
          meta: {
            title: 'Systems',
            activeMenu: 'systems',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'unauthorized',
          name: 'unauthorized',
          component: Unauthorized,
          meta: {
            title: 'unauthorized',
            activeMenu: 'unauthorized',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'system/:id',
          name: 'editsystem',
          component: AddSystem,
          meta: {
            title: 'editsystem',
            activeMenu: 'systems',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'menus',
          name: 'menus',
          component: MenuList,
          meta: {
            title: 'menu',
            activeMenu: 'menus',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'menu',
          name: 'menu',
          component: Menu,
          meta: {
            title: 'menu',
            activeMenu: 'menus',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'menu/:id',
          name: 'menuedit',
          component: Menu,
          meta: {
            title: 'menu',
            activeMenu: 'menus',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'rolemenus/:id',
          name: 'rolemenus',
          component: RoleMenuList,
          meta: {
            title: 'rolemenus',
            activeMenu: 'roles',
            hideInMenu: true,
            notCache: true
          }
        }, {
          path: 'addsystem',
          name: 'addsystem',
          component: AddSystem,
          meta: {
            title: 'AddSystem',
            activeMenu: 'systems',
            hideInMenu: true,
            notCache: true
          }
        }
      ]
    },
    {
      path: '/*',
      name: 'Error_404',
      meta: {
        hideInMenu: true
      },
      component: NotFound
    },
    {
      path: '*',
      meta: { requiresAuth: process.env.ENABLE_AUTH },
      redirect: '/selectapp'
    }

  ]
})
