<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{ name: 'dashboard' }">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{ name: 'roles' }">角色列表</BreadcrumbItem>
            <BreadcrumbItem>{{ breadtitle }}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
            <Card>
                <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                    <FormItem label="角色名" prop="name">
                        <Input v-model="formData.name" placeholder="Enter your name"></Input>
                    </FormItem>
                    <FormItem label="所属系统" prop="system">
                        <SystemListSelect :value.sync="formData.system" :disableEdit.sync="disableType">
                        </SystemListSelect>
                    </FormItem>
                    <!--FormItem label="角色类型" prop="roletype">
                            <Select v-model="formData.roletype" placeholder="角色类型" :disabled="disableType">
                                <Option v-for="item in formData.roletypelist" :value="item.value" :key="item.value">{{ item.label }}</Option>
                            </Select>
                        </FormItem-->
                    <FormItem label="描述" prop="desc">
                        <Input v-model="formData.desc" type="textarea" :autosize="{ minRows: 2, maxRows: 5 }"
                            placeholder="Enter ..."></Input>
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
    data() {
        return {
            breadtitle: '角色添加',
            disableSystem: false,
            disableType: false,
            subDisable: false,
            formData: {
                id: 0,
                name: '',
                desc: '',
                system: '',
                roletype: 1,
                roletypelist: [
                    {
                        value: 1,
                        label: '主账号角色'
                    },
                    {
                        value: 2,
                        label: '子账号角色'
                    }
                ],
                systemlist: [
                ]
            },
            ruleValidate: {
                name: [
                    { required: true, message: 'The name cannot be empty', trigger: 'blur' }
                ],
                system: [
                    { required: true, message: 'Please select the system', trigger: 'change' }
                ],
                roletype: [
                    { required: true, type: 'number', message: 'Please select the role type', trigger: 'change' }
                ]
            }
        }
    },
    created() {
        this.queryData();
    },
    methods: {
        handleSubmit(name) {
            this.$refs[name].validate((valid) => {
                if (valid) {
                    this.subDisable = true;

                    if (this.id > 0) {
                        let params = {
                            id: parseInt(this.id),
                            name: this.formData.name,
                            desc: this.formData.desc
                        }
                        this.$axios.post(this.$API.PERMISSION_ROLE_EDIT, params)
                            .then(response => {
                                console.log(response);
                                if (response.data && response.data.rc == '0') {
                                    this.$Message.success('成功!');
                                } else {
                                    this.$Message.error('失败!');
                                }
                                this.subDisable = false;
                            });

                    } else {
                        let params = {
                            name: this.formData.name,
                            sys: this.formData.system,
                            type: parseInt(this.formData.roletype),
                            desc: this.formData.desc,
                        }
                        this.$axios.post(this.$API.PERMISSION_ROLE_ADD, params)
                            .then(response => {
                                console.log(response);
                                if (response.data && response.data.rc == '0') {
                                    this.$Message.success('成功!');
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
        handleReset(name) {
            this.$refs[name].resetFields();
        },
        queryData() {
            let id = this.$route.params.id;
            if (id > 0) {
                this.breadtitle = '角色编辑';
                this.disableSystem = true;
                this.disableType = true;
                this.id = id;

                let params = {
                    id: parseInt(id)
                }
                this.$axios.post(this.$API.PERMISSION_ROLE_SHOW, params)
                    .then(response => {
                        console.log(response);
                        if (response.data && response.data.rc == '0') {

                            this.formData.name = response.data.name;
                            this.formData.system = response.data.system;
                            this.formData.desc = response.data.desc;
                            this.formData.roletype = response.data.type;

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

