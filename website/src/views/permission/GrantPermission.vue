<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'roles'}">角色列表</BreadcrumbItem>
            <BreadcrumbItem>角色授权</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
                <Card>
                  <Form ref="formData" :label-width="80" style="width: 800px">
                        <FormItem label="角色" prop="roleName">
                            <span>{{ roleName }}</span>
                        </FormItem>
                        <FormItem label="所属系统" prop="roleSystem">
                            <span>{{ roleSystem }}</span>
                        </FormItem>
                        <FormItem label="权限选择" prop="permissions">
                            <Transfer
                            :data="allPermissions"
                            :target-keys="selectPermissions"
                            :list-style="listStyle"
                            filterable
                            :filter-method="filterMethod"
                            :titles="titles"
                            @on-change="handleChange1"></Transfer>
                        </FormItem>


                        <FormItem>
                            <Button type="primary" @click="handleSubmit('formData')" :disabled="subDisable">保存</Button>
                            <Button @click="handleReset('formData')" style="margin-left: 8px">重置</Button>
                        </FormItem>
                    </Form>
                </Card>
        </div>

    </div>
</template>
<script>
export default {
        data () {
            return {
                roleId:0,
                roleName:'',
                roleSystem : '',
                subDisable: false,
                allPermissions: [],
                listStyle: {
                    width: '250px',
                    height: '300px'
                },
                titles : ["所有权限","已选权限"],
                selectPermissions: []
            }
        },
        created() {
            this.queryData();
        },
        methods: {
            handleSubmit (name) {
                this.subDisable = true;
                if(this.roleId > 0) {
                    let params = {
                        id:parseInt(this.roleId),
                        permissions:this.selectPermissions
                    }
                     this.$axios
                         .post(this.$API.PERMISSION_ROLE_EDIT, params)
                         .then(response => {
                            console.log(response);
                            if(response.data && response.data.rc == '0') {
                                this.$Message.success('成功!');
                            } else if (response.data.rc == '-3003') {
                                this.$Message.error('未登陆!');
                            } else if (response.data.rc == '-4001') {
                                this.$Message.error('无权访问!');
                            } else {
                                this.$Message.error('失败!');
                            }
                            this.subDisable = false;
                        });
                }
            },
            handleReset (name) {
                this.$refs[name].resetFields();
            },
            queryData() {
                this.roleId = this.$route.params.id;
                this.subDisable = true;
                if(this.roleId > 0) {
                    let params = {
                        id:parseInt(this.roleId)
                    }
                    this.$axios
                     .post(this.$API.PERMISSION_ROLE_SHOW, params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {
                            this.roleName = response.data.name;
                            this.roleSystem = response.data.system;
                            if(response.data.permissions) {
                                for(let p in response.data.permissions) {
                                    this.selectPermissions.push(response.data.permissions[p].permission);
                                }
                            }

                            this.getAllPermissions(this.roleSystem);

                        } else if (response.data.rc == '-3003') {
                            this.$Message.error('未登陆!');
                        } else if (response.data.rc == '-4001') {
                            this.$Message.error('无权访问!');
                        } else {
                            this.$Message.error('错误!');
                        }
                        this.subDisable = false;
                    });
                    
                } else {
                    this.$Message.error('错误!');
                }

            },
            handleChange1 (newTargetKeys, direction, moveKeys) {
                this.selectPermissions = newTargetKeys;
            },
            getAllPermissions (sys) {
                let params = {
                    sys : sys,
                    size : 500
                };
                 this.$axios
                 .post(this.$API.PERMISSION_PERM_LIST, params)
                 .then(response => {
                        
                        if(response.data && response.data.rc == '0') {
                            this.allPermissions = [];
                            for(let i in response.data.list) {
                                let pname = response.data.list[i].name;
                                let permission = response.data.list[i].permission;

                                let o = {
                                    key: permission,
                                    label: pname + '('+permission+')',
                                    description: '',
                                    disable: false
                                }
                                this.allPermissions.push(o);
                            }
                        }

                 });
            },
            filterMethod (data, query) {
                return data.label.indexOf(query) > -1;
            }
        }
    }
</script>

