// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'

import iView from 'iview'

import ElementUI from 'element-ui'
import 'element-ui/lib/theme-default/index.css'

import App from './App'
import router from './router'

import store from './vuex/store'
import API from '@/config/api'
import Bus from '@/utils/bus';

import MainMenu from '@/components/Menu'
import HeaderBar from '@/components/Header'
import VueCookie from 'vue-cookie'
import VueResource from 'vue-resource'

import uploader from 'vue-simple-uploader'
import VDistpicker from 'v-distpicker'
import Schart from 'vue-schart'

import './axios'

Vue.config.productionTip = false

Vue.use(ElementUI)
Vue.use(iView)

Vue.use(VueCookie);
Vue.use(VueResource);
Vue.use(uploader)
// 公共组件
Vue.component('main-menu', MainMenu);
Vue.component('header-bar', HeaderBar);
Vue.component('v-distpicker', VDistpicker)
Vue.component('schart', Schart)

import 'iview/dist/styles/iview.css'

// 属性扩充
Vue.prototype.$API = API;

router.beforeEach((to, from, next) => {
    iView.LoadingBar.start();
    if (to.params.appId) {
        console.log('router app id:', to.params.appId);
        API.APPID = to.params.appId;
    }

    if (to.matched.some(record => record.meta.requiresAuth)) {
        if (to.path == '/login') {
            VueCookie.delete('aproton_session')
            sessionStorage.removeItem('userAccount')
        } else if (!sessionStorage.getItem('userAccount')) {
            console.log('not login')

            let url = '/web/passport/session';
            Vue.http.get(url).then(function (response) {
                let m = response.body;
                if (m.error == 'success' || m.rc == '0') {
                    sessionStorage.setItem('userAccount', m.email)
                    next();
                } else {
                    if (process.env.NODE_ENV != 'development') {
                        iView.LoadingBar.finish();
                    }
                    sessionStorage.removeItem('userAccount')
                    next({ path: '/login' })
                }
            }, function () {
                if (process.env.NODE_ENV != 'development') {
                    iView.LoadingBar.finish();
                }
                sessionStorage.removeItem('userAccount')
                next({ path: '/login' })
            });
        } else {
            next()
        }
    } else {
        next() // 确保一定要调用 next()
    }

    if (to.matched.some(record => record.meta.mainMenuId)) {
        var menu = to.matched.find(record => record.meta.mainMenuId)
        Bus.mainMenuId = menu.meta.mainMenuId
        Bus.$emit('menu-update', menu.meta.mainMenuId)
    } else {
        next()
    }
});

router.afterEach(to => {
    iView.LoadingBar.finish()
    window.scrollTo(0, 0)
});

/* eslint-disable no-new */
new Vue({
    el: '#app',
    router,
    store: store,
    template: '<App/>',
    components: { App }
})
