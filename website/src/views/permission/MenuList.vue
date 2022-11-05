<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{ name: 'dashboard' }">Home</BreadcrumbItem>
            <BreadcrumbItem>菜单列表</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
            <Card>

                <div class="form-search">

                    <Form ref="formSearch" :model="formSearch" inline>
                        <FormItem>
                            <Button type="info" icon="md-add" to='menu'>Add</Button>
                        </FormItem>
                        <FormItem>
                            <Input type="text" v-model="formSearch.name" placeholder="搜索" :disabled="searchDisable"
                                style="width: 300px">
                            </Input>
                        </FormItem>
                        <!--FormItem>
                <Input
                    type="text"
                    v-model="formSearch.type"
                    placeholder="类型"
                    :disabled="searchDisable"
                    style="width: 100px"
                >
                </Input>
            </FormItem-->
                        <FormItem>
                            <SystemListSelect :value.sync="formSearch.system"></SystemListSelect>
                        </FormItem>
                        <FormItem>
                            <Button type="primary" icon="md-search" :disabled="searchDisable"
                                @click="onSearchSubmit">搜索</Button>
                        </FormItem>
                    </Form>


                </div>

                <div>
                    <Table :loading=loading :data="tableData1" :columns="tableColumns1" stripe>
                        <template slot-scope="{ row, index }" slot="action">
                            <Button type="primary" size="small" style="margin-right: 5px"
                                :to="{ name: 'menuedit', params: { id: row.id } }">编辑</Button>
                            <Button type="error" size="small" style="margin-right: 5px"
                                @click="onDel(row.id)">删除</Button>
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
import SystemListSelect from '@/components/SystemListSelect';
export default {
    components: {
        SystemListSelect
    },
    data() {
        return {
            tableData1: [],
            loading: false,
            page: 1,
            pagesize: 20,
            total: 0,
            searchDisable: true,
            formSearch: {
                name: '',
                system: '',
                type: '',
                SystemList: [
                ]
            },
            tableColumns1: [
                {
                    title: 'ID',
                    key: 'id'
                },
                {
                    title: '标题',
                    key: 'title'
                },
                {
                    title: 'url',
                    key: 'url'
                },
                {
                    title: '父级菜单ID',
                    key: 'pid'
                },
                {
                    title: '所属系统',
                    key: 'system'
                },
                {
                    title: '属性',
                    key: 'attrs'
                },
                {
                    title: '描述',
                    key: 'desc'
                },
                {
                    title: '添加时间',
                    key: 'createdAt',
                },
                {
                    title: '操作',
                    slot: 'action',
                }
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
                page: this.page,
                sys: this.formSearch.system,
                title: this.formSearch.name,
                type: this.formSearch.type,
                size: this.pagesize
            }
            this.$axios.post(this.$API.PERMISSION_MENU_LIST, params)
                .then(response => {
                    console.log(response);
                    if (response.data && response.data.rc == '0') {
                        this.total = response.data.total;
                        this.tableData1 = response.data.list;
                    } else {
                        this.$Message.error('错误!');
                    }
                    this.loading = false;
                    this.searchDisable = false;
                });
        },
        onEdit(id) {
            console.log(id);
        },
        onDel(id) {
            if (confirm('你确定要删除吗？')) {
                let params = {
                    id: parseInt(id),
                }
                this.$axios.post(this.$API.PERMISSION_MENU_DEL, params)
                    .then(response => {
                        console.log(response);
                        if (response.data && response.data.rc == '0') {
                            this.$Message.success('成功!');
                            this.fetchData();
                        } else {
                            this.$Message.error('失败!');
                        }
                    });
            } else {
                // alert('no');
            }
        },
        onGrant(id) {
            console.log(id);
        },
        onSearchSubmit() {
            this.fetchData();
        },
        changePage(p) {
            // The simulated data is changed directly here, and the actual usage scenario should fetch the data from the server
            this.page = p;
            this.fetchData();
        }
    }
}
</script>
