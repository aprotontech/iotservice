<template>
<div>
      <Breadcrumb style="margin:10px">
             <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem>系统列表</BreadcrumbItem>
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
                        to='addsystem'
                    >Add</Button>
            </FormItem>
          
        </Form>


    </div>

    <div>
    <Table :loading=loading :data="tableData1" :columns="tableColumns1" stripe>
        <template slot-scope="{ row, index }" slot="action">
            <Button type="primary" size="small" style="margin-right: 5px" :to="{name:'editsystem', params:{ id:row.id }}" >编辑</Button>
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
export default {
        data () {
            return {
                tableData1: [],
                loading : false,
                page: 1,
                pagesize: 20,
                total: 0,
                formSearch: {
                    account: ''
                },
                tableColumns1: [
                    {
                        title: 'ID',
                        key: 'id'
                    },
                    {
                        title: '名字',
                        key: 'name'
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
                let params = {
                        page: this.page,
                        size: this.pagesize
                    }
                this.$axios
                     .post(this.$API.PERMISSION_SYSTEM_LIST, params)
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
                    });
            },
            onEdit(id) {
                console.log(id);
            },
            onSearchSubmit() {
                console.log(this.formSearch);
            },
            changePage (p) {
                // The simulated data is changed directly here, and the actual usage scenario should fetch the data from the server
                this.page = p;
                this.fetchData();
            }
        }
    }
</script>
