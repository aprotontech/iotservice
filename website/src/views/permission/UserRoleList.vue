<template>
<div>
      <Breadcrumb style="margin:10px">
             <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'accountlist'}">用户列表</BreadcrumbItem>
            <BreadcrumbItem>{{ username }} ({{ useraccount }})</BreadcrumbItem>
        </Breadcrumb>

    <div class="content">
        <Card>
               
     <div class="form-search">
        
        <Form
            inline
        >
            <FormItem>
                 <Button
                        type="info"
                        icon="md-add"
                        :to="{name:'adduserrole', params:{ id:userid}}"
                    >添加角色</Button>
            </FormItem>
       
        </Form>


    </div>

    <div>
    <Table :loading=loading :data="tableData1" :columns="tableColumns1" stripe>
        <template slot-scope="{ row, index }" slot="action">
            <Button type="primary" size="small" style="margin-right: 5px" :to="{name:'grantdata', params:{ id:row.id }}" v-if="row.dataRule!='*' && row.dataRule!=''">数据规则</Button>
            <Button type="error" size="small" style="margin-right: 5px" @click="onDel(row.id)">删除</Button>
        </template>
    </Table>
    </div>


    </Card>

    </div>

    </div>
</template>
<script>
export default {
        data () {
            return {
                tableData1: [],
                loading : false,
                username: "",
                useraccount: "",
                userid : 0,
                tableColumns1: [
                    {
                        title: '角色ID',
                        key: 'roleId'
                    },
                    {
                        title: '角色名',
                        key: 'role'
                    },
                    {
                        title : '所属系统',
                        key : 'system'
                    },
                    {
                        title : '数据规则',
                        key : 'dataRule'
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
                this.loading = true;
                
                this.userid = this.$route.params.id;
                let params = {
                        userId: parseInt(this.userid),
                    }
                this.$axios
                     .post(this.$API.PERMISSION_USERROLE_SHOW, params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {
                            this.useraccount = response.data.account;
                            this.username = response.data.name;
                            this.tableData1 = response.data.roles;
                        } else {
                            this.$Message.error('错误!');
                        }
                        this.loading = false;
                    });

            },
            onDel(id) {
                if(confirm('你确定要删除吗？')) {
                        let params = {
                            id:parseInt(id),
                        }
                        this.$axios
                             .post(this.$API.PERMISSION_USERROLE_DEL, params)
                             .then(response => {
                                console.log(response);
                                if(response.data && response.data.rc == '0') {
                                    this.$Message.success('成功!');
                                    this.fetchData();
                                } else {
                                    this.$Message.error('失败!');
                                }
                            });
                } else {
                    // alert('no');
                }
            }
        }
    }
</script>
