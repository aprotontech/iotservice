"use strict";


import axios from "axios";
import router from './router'

// Full config:  https://github.com/axios/axios#request-config
// axios.defaults.baseURL = process.env.baseURL || process.env.apiUrl || '';
// axios.defaults.headers.common['Authorization'] = AUTH_TOKEN;
// axios.defaults.headers.post['Content-Type'] = 'application/x-www-form-urlencoded';

let config = {
  // baseURL: process.env.baseURL || process.env.apiUrl || ""
  timeout: 60 * 1000, // Timeout
  // withCredentials: true, // Check cross-site Access-Control
};

const _axios = axios.create(config);

axios.interceptors.request.use(
  function (config) {
    // Do something before request is sent
    return config;
  },
  function (error) {
    // Do something with request error
    return Promise.reject(error);
  }
);

// Add a response interceptor
axios.interceptors.response.use(
  function (response) {
    // Do something with response data
    //console.log('interceptor', response);
    if (response && response.data && response.data.rc == "401") { // user is not login
      if (window.location.href.indexOf('/login') == -1) {
        console.log('User is not login, url: ' + window.location.href)
        router.push({ path: '/login', query: { 'redirect_to': window.location.href } })
        return;
      }
    }
    return response;
  },
  function (error) {
    // Do something with response error

    return Promise.reject(error);
  }
);

export default axios;
