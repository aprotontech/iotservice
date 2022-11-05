<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{ name: 'dashboard' }">Home</BreadcrumbItem>
            <BreadcrumbItem>用户列表</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
            <Card>

                <div class="form-search">
                    <Form ref="formSearch" :model="formSearch" inline>
                        <FormItem>
                            <Input type="text" v-model="formSearch.account" @keyup.enter.native="onSearchSubmit()"
                                placeholder="搜索账号" style="width: 300px">
                            </Input>
                        </FormItem>
                        <FormItem>
                            <Button type="primary" icon="md-search" :disabled="searchDisable"
                                @click="onSearchSubmit">搜索</Button>
                        </FormItem>
                        <FormItem>
                            <Button style="margin-right: 5px" type="primary" :to="{ name: 'newaccount' }">新建</Button>
                        </FormItem>
                    </Form>
                </div>
                <input id="foo" value="" v-show="0"></input>

                <div>
                    <Table :loading=loading :data="tableData1" :columns="tableColumns1" stripe>
                        <template slot-scope="{ row }" slot="account">
                            <strong>{{ row.account }}</strong>
                        </template>
                        <template slot-scope="{ row, index }" slot="action">
                            <Button type="primary" size="small" style="margin-right: 5px"
                                :to="{ name: 'userroles', params: { id: row.id } }">角色</Button>
                            <Button type="warning" size="small" style="margin-right: 5px"
                                @click='randNewPassword(row)'>修改密码</Button>
                        </template>
                    </Table>
                </div>

                <div style="margin: 10px;overflow: hidden">
                    <div style="float: right;">
                        <Page :total=total :page-size=pagesize :current=page @on-change="changePage" show-elevator
                            show-total size="small"></Page>
                    </div>
                </div>

            </Card>

        </div>

    </div>
</template>
<script>
import Clipboard from 'clipboard';
import { upDownState, ApiMessage, formatDate } from '@/utils/util'
export default {
    data() {
        return {
            tableData1: [],
            loading: false,
            page: 1,
            pagesize: 20,
            total: 0,
            searchDisable: true,
            formSearch: {
                account: '',
            },
            tableColumns1: [
                {
                    title: 'ID',
                    key: 'id'
                },
                {
                    title: '昵称',
                    key: 'name'
                },
                {
                    title: '邮箱',
                    key: 'email'
                },
                {
                    title: '状态',
                    key: 'state',
                    render: (h, params) => {
                        const row = params.row;
                        const color = row.state === 1 ? 'success' : 'warning';
                        const text = row.state === 1 ? '启用' : '禁用';

                        return h('Tag', {
                            props: {
                                type: 'dot',
                                color: color
                            }
                        }, text);
                    }
                },
                {
                    title: '添加时间',
                    key: 'createdAt',
                },
                {
                    title: '修改时间',
                    key: 'updatedAt',
                },
                {
                    title: '操作',
                    slot: 'action',
                },
            ]
        }
    },
    created() {
        this.fetchData();
    },
    methods: {
        fetchData() {
            this.loading = true;
            this.searchDisable = true;
            let params = {
                account: this.formSearch.account,
                page: this.page,
                size: this.pagesize
            }
            this.$axios.post(this.$API.PERMISSION_ACCOUNT_LIST, params)
                .then(response => {
                    console.log(response);
                    var m = response.data
                    if (m.rc == '0') {
                        this.total = m.total;
                        this.tableData1 = m.list;
                    } else {
                        ApiMessage(this, "get account list failed", 'error')
                    }
                    this.loading = false;
                    this.searchDisable = false;
                });
        },
        showrole(id) {
            console.log(id);
        },
        onSearchSubmit() {
            this.fetchData();
        },
        randNewPassword(row) {
            console.log(row)
            var returnStr = "",
                charStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
            for (var i = 0; i < 16; i++) {
                var index = Math.round(Math.random() * (charStr.length - 1));
                returnStr += charStr.substring(index, index + 1);
            }
            if (!confirm('是否修改用户(' + row.account + ')的密码？ 新密码是： \n' + returnStr)) {
                return;
            }
            let userPrams = {
                email: row.account,
                password: returnStr
            }
            this.$axios.post(this.$API.PERMISSION_ACCOUNT_RESET_PASSWD, userPrams)
                .then((response) => {
                    let res = response.data;
                    if (res.rc == 0) {
                        ApiMessage(this, '修改成功', 'success');
                        alert('新密码是： ' + returnStr);
                    } else {
                        ApiMessage(this, res.err, 'error')
                    }
                }).catch((error) => {
                    ApiMessage(this, error.toString(), 'error')
                })

        },
        changePage(p) {
            // The simulated data is changed directly here, and the actual usage scenario should fetch the data from the server
            this.page = p;
            this.fetchData();
        }
    }
}
</script>
