<template>
<div>
      <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'roles'}">角色列表</BreadcrumbItem>
            <BreadcrumbItem>角色菜单</BreadcrumbItem>
        </Breadcrumb>

    <div class="content">
        <Card>

    <div>
     <Tree :data="data4" show-checkbox multiple ref="tree" @on-check-change="choice"></Tree>
    </div>
    <div style="margin: 50px;overflow: hidden">
     <Button type="primary" @click="handleSubmit('formData')" :disabled="subDisable">保存</Button>
     </div>

    </Card>

    </div>

    </div>
</template>
<script>
export default {
        data () {
            return {
                subDisable: false,
                id:0,
                selectData : [],
                 data4: [
                    
                ],
                loading : false,
            }
        },
        created() {
            this.fetchData();
        },
        methods: {
            fetchData () {
                let id = this.$route.params.id;
                this.id = id;
                this.loading = true;
                this.subDisable = true;
                let params = {
                        id: parseInt(this.id),
                    }
                this.$axios
                     .post(this.$API.PERMISSION_ROLEMENU_LIST, params)
                     .then(response => {
                        console.log(response.data.menus);

                        if(response.data && response.data.rc == '0') {
                                this.data4.push({
                                    title : response.data.menus.title,
                                    expand : true,
                                    selected : true,
                                    key: 0,
                                    children : response.data.menus.children,
                                });


                            this.$nextTick(function() {
                                this.selectData = this.$refs.tree.getCheckedAndIndeterminateNodes();
                            })
                        } else if (response.data.rc == '-3003') {
                            this.$router.push('/login');
                        } else if (response.data.rc == '403') {
                            this.$router.push('/unauthorized');
                        }
                        this.loading = false;
                        this.subDisable = false;
                    });
            },
            handleSubmit(form) {
                let mid = [];
                if (this.selectData) {
                    for(let i in this.selectData) {
                        let id = this.selectData[i].key;
                        if(id > 0) {
                            mid.push(id);
                        }
                    }
                }
                this.subDisable = true;
                let params = {
                        roleId: parseInt(this.id),
                        menus: mid
                    }
                this.$axios
                     .post(this.$API.PERMISSION_ROLEMENU_ADD, params)
                     .then(response => {
                        if(response.data && response.data.rc == '0') {
                            this.$Message.success('成功!');
                        } else if (response.data.rc == '-3003') {
                            this.$router.push('/login');
                        } else if (response.data.rc == '-4001') {
                            this.$router.push('/unauthorized');
                        }
                        this.subDisable = false;
                    });

            },
            choice() {
                this.selectData = this.$refs.tree.getCheckedAndIndeterminateNodes();
                let mid = [];
                if (this.selectData) {
                    for(let i in this.selectData) {
                        let id = this.selectData[i].key;
                        if(id > 0) {
                            mid.push(id);
                        }
                    }
                }
                console.log(mid);
            }
        }
    }
</script>
