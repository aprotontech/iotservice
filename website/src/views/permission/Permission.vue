<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'permissions'}">权限列表</BreadcrumbItem>
            <BreadcrumbItem>{{breadtitle}}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
                <Card>
                  <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                        <FormItem label="权限标示" prop="permission" >
                            <Input v-model="formData.permission" placeholder="example：ota_view" :disabled="disablePermission"></Input>
                        </FormItem>
                         <FormItem label="权限名" prop="name" >
                            <Input v-model="formData.name" placeholder="Enter name"></Input>
                        </FormItem>
                        <FormItem label="所属系统" prop="system">
                            <SystemListSelect :value.sync="formData.system" :disableEdit="disableSystem"></SystemListSelect>
                        </FormItem>
                        <FormItem label="描述" prop="desc">
                            <Input v-model="formData.desc" type="textarea" :autosize="{minRows: 2,maxRows: 5}" placeholder="Enter ..."></Input>
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
import SystemListSelect from '@/components/SystemListSelect';
export default {
    components: {
            SystemListSelect
        },
        data () {
            return {
                breadtitle:'权限添加',
                disableSystem: false,
                disablePermission: false,
                subDisable: false,
                permissionId: 0,
                formData: {
                    permission: '',
                    name: '',
                    desc: '',
                    system: '',
                    systemlist:[
                        {
                            value:'aproton',
                            label:'aproton',
                        },
                        {
                            value:'admin',
                            label:'permission admin'
                        }
                    ]
                },
                ruleValidate: {
                    permission: [
                        { required: true, message: 'The permission cannot be empty', trigger: 'blur' }
                    ],
                    name: [
                        { required: true, message: 'The name cannot be empty', trigger: 'blur' }
                    ],
                    system: [
                        { required: true, message: 'Please select the system', trigger: 'change' }
                    ],
                }
            }
        },
        created() {
            this.queryData();
        },
        methods: {
            handleSubmit (name) {
                this.$refs[name].validate((valid) => {
                    if (valid) {
                        this.subDisable = true;
                        if(this.permissionId <= 0) {
                            let params = {
                                permission:this.formData.permission,
                                sys:this.formData.system,
                                name:this.formData.name,
                                description: this.formData.desc
                            }
                             this.$axios
                                 .post(this.$API.PERMISSION_PERM_ADD, params)
                                 .then(response => {
                                    console.log(response);
                                    if(response.data && response.data.rc == '0') {
                                        this.$Message.success('成功!');
                                    } else if (response.data.rc == '-3003') {
                                        this.$Message.error('未登陆!');
                                    } else if (response.data.rc == '-4001') {
                                        this.$Message.error('无权访问!');
                                     } else if (response.data.rc == '-5004') {
                                        this.$Message.error('权限标示已存在!');
                                    } else {
                                        this.$Message.error('失败!');
                                    }
                                    this.subDisable = false;
                                });

                        } else {
                            let params = {
                                id:parseInt(this.permissionId),
                                name:this.formData.name,
                                description: this.formData.desc
                            }
                             this.$axios
                                 .post(this.$API.PERMISSION_PERM_EDIT, params)
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

                    } else {
                        // this.$Message.error('Fail!');
                    }
                })
            },
            handleReset (name) {
                this.$refs[name].resetFields();
            },
            queryData() {
                let id = this.$route.params.id;
                if(id > 0) {
                    this.permissionId = id;
                    this.breadtitle = '权限编辑';
                    this.disableSystem = true;
                    this.disablePermission = true;
                    this.subDisable = true;

                    let params = {
                        id:parseInt(id)
                    }
                    this.$axios
                     .post(this.$API.PERMISSION_PERM_SHOW, params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {

                            this.formData.permission = response.data.permission;
                            this.formData.name = response.data.name;
                            this.formData.system = response.data.system;
                            this.formData.desc = response.data.description;

                        } else if (response.data.rc == '-3003') {
                            this.$Message.error('未登陆!');
                        } else if (response.data.rc == '-4001') {
                            this.$Message.error('无权访问!');
                        } else {
                            this.$Message.error('错误!');
                        }
                        this.subDisable = false;
                    });

                }
            }
        }
    }
</script>

