<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{ name: 'dashboard' }">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{ name: 'systems' }">系统列表</BreadcrumbItem>
            <BreadcrumbItem>{{ breadtitle }}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
            <Card>
                <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                    <FormItem label="系统名" prop="name">
                        <Input v-model="formData.name" placeholder="不能重复" :disabled="disableEdit"></Input>
                    </FormItem>

                    <FormItem label="描述" prop="desc">
                        <Input v-model="formData.desc" type="textarea" :autosize="{ minRows: 2, maxRows: 5 }"
                            placeholder=""></Input>
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
    data() {
        return {
            id: 0,
            breadtitle: '系统添加',
            disableEdit: false,
            subDisable: false,
            formData: {
                name: '',
                desc: '',
            },
            ruleValidate: {
                name: [
                    { required: true, message: 'The name cannot be empty', trigger: 'blur' }
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
                            desc: this.formData.desc
                        }
                        this.$axios.post(this.$API.PERMISSION_SYSTEM_EDIT, params)
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
                            desc: this.formData.desc,
                        }
                        this.$axios.post(this.$API.PERMISSION_SYSTEM_ADD, params)
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
            this.id = this.$route.params.id;
            if (this.id > 0) {
                this.breadtitle = '系统编辑';
                this.disableEdit = true;
                this.subDisable = true;

                let params = {
                    id: parseInt(this.id)
                }
                this.$axios.post(this.$API.PERMISSION_SYSTEM_SHOW, params)
                    .then(response => {
                        console.log(response);
                        if (response.data && response.data.rc == '0') {

                            this.formData.name = response.data.name;
                            this.formData.desc = response.data.desc;

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

