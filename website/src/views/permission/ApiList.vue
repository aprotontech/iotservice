<template>
<div>
      <Breadcrumb style="margin:10px">
             <BreadcrumbItem to="/dashboard">Home</BreadcrumbItem>
            <BreadcrumbItem>API列表</BreadcrumbItem>
        </Breadcrumb>

    <div class="content">
        <Card>
               
     <div class="form-search">
        
        <Form
            ref="formSearch"
            :model="formSearch"
            inline
        >
            <FormItem>
                 <Button
                        type="info"
                        icon="md-add"
                        :to="{name:'apiadd', query:{ p: formSearch.permission, system:formSearch.system }}"
                    >Add</Button>
            </FormItem>
            <FormItem>
                <Input
                    type="text"
                    v-model="formSearch.name"
                    placeholder="搜索接口地址"
                    style="width: 300px"
                />
            </FormItem>
            <FormItem>
                <Input
                    type="text"
                    v-model="formSearch.permission"
                    placeholder="权限标示"
                    style="width: 300px"
                />
            </FormItem>
            <FormItem>
                <SystemListSelect :value.sync="formSearch.system"></SystemListSelect>
            </FormItem>
          <FormItem>
            <Button
                type="primary"
                icon="md-search"
                :disabled="searchDisable"
                @click="onSearchSubmit"
            >搜索</Button>
        </FormItem>
        </Form>


    </div>

    <div>
    <Table :loading=loading :data="tableData1" :columns="tableColumns1" stripe>
        <template slot-scope="{ row, index }" slot="action">
            <Button type="primary" size="small" style="margin-right: 5px" :to="{name:'apiedit', params:{ id:row.id }}">编辑</Button>
            <Button type="error" size="small" style="margin-right: 5px" @click="onDel(row.id)">删除</Button>
        </template>
    </Table>
    </div>

    <div style="margin: 10px;overflow: hidden">
        <div style="float: right;">
            <Page :total=total :page-size=pagesize :current=page @on-change="changePage" show-elevator show-total size="small"></Page>
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
        data () {
            return {
                tableData1: [],
                loading : false,
                page: 1,
                pagesize: 20,
                total: 0,
                searchDisable: true,
                formSearch: {
                    name: '',
                    permission:'',
                    system: '',
                    systemlist:[
                        {
                            value:'aproton',
                            label:'aproton',
                        },
                        {
                            value:'padmin',
                            label:'permission admin',
                        }
                    ],
                },
                tableColumns1: [
                    {
                        title: 'ID',
                        key: 'id'
                    },
                    {
                        title: '接口地址',
                        key: 'uri'
                    },
                    {
                        title: '请求方法',
                        key: 'method'
                    },
                    {
                        title: '请求域名',
                        key: 'domain'
                    },
                    {
                        title: '超时时间',
                        key: 'timeout'
                    },
                    {
                        title: '权限标示',
                        key: 'permission'
                    },
                    {
                        title: '所属系统',
                        key: 'system'
                    },
                    {
                        title: '服务地址',
                        key: 'serviceUrl'
                    },
                    {
                        title: '添加时间',
                        key: 'createdAt',
                    },
                    {
                        title: '操作',
                        slot : 'action',
                    }
                ]
            }
        },
        created() {
            this.fetchData();
        },
        methods: {
            fetchData () {

                this.formSearch.permission = this.$route.query.p;
                this.formSearch.system = this.$route.query.system;

                this.loading = true;
                this.searchDisable = true;
                let params = {
                        permission: this.formSearch.permission,
                        uri:this.formSearch.name,
                        page: this.page,
                        sys:this.formSearch.system,
                        size: this.pagesize
                    }
                this.$axios
                     .post('/admin/api/list', params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {
                            this.total = response.data.total;
                            this.tableData1 = response.data.list;
                        } else if (response.data.rc == '-3003') {
                            this.$router.push('/login');
                        } else if (response.data.rc == '-4001') {
                            this.$router.push('/unauthorized');
                        }
                        this.loading = false;
                        this.searchDisable = false;
                    });

            },
            onEdit(id) {
                console.log(id);
            },
            onDel(id) {
                if(confirm('你确定要删除吗？')) {
                    let params = {
                        id:parseInt(id),
                    }
                    this.$axios
                         .post('/admin/api/del', params)
                         .then(response => {
                            console.log(response);
                            if(response.data && response.data.rc == '0') {
                                this.$Message.success('成功!');
                                this.fetchData();
                            } else if (response.data.rc == '-3003') {
                                this.$Message.error('未登陆!');
                            } else if (response.data.rc == '-4001') {
                                this.$Message.error('无权访问!');
                            } else {
                                this.$Message.error('失败!');
                            }
                        });
                } else {
                    // alert('no');
                }
            },
            onGrant(id) {

            },
            onSearchSubmit() {
                this.fetchData();
            },
            changePage (p) {
                // The simulated data is changed directly here, and the actual usage scenario should fetch the data from the server
                this.page = p;
                this.fetchData();
            }
        }
    }
</script>
