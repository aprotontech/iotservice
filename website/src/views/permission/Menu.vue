<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{ name: 'dashboard' }">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{ name: 'menus' }">菜单列表</BreadcrumbItem>
            <BreadcrumbItem>{{ breadtitle }}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
            <Card>
                <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                    <FormItem label="菜单标题" prop="title">
                        <Input v-model="formData.title" placeholder=""></Input>
                    </FormItem>
                    <FormItem label="地址" prop="url">
                        <Input v-model="formData.url" placeholder="Enter url"></Input>
                    </FormItem>
                    <FormItem label="所属系统" prop="system">
                        <SystemListSelect :value.sync="formData.system" :disableEdit="disableSystem"></SystemListSelect>
                    </FormItem>
                    <FormItem label="父菜单ID" prop="pid">
                        <Input v-model="formData.pid" placeholder=""></Input>
                    </FormItem>
                    <FormItem label="分类" prop="type">
                        <Input v-model="formData.type" placeholder=""></Input>
                    </FormItem>
                    <FormItem label="排序" prop="sort">
                        <Input v-model="formData.sort" placeholder=""></Input>
                    </FormItem>
                    <FormItem label="属性" prop="attrs">
                        <Input v-model="formData.attrs" type="textarea" :autosize="{ minRows: 2, maxRows: 5 }"
                            placeholder="json"></Input>
                    </FormItem>
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
            breadtitle: '菜单添加',
            disableSystem: false,
            disableMenu: false,
            subDisable: false,
            menuId: 0,
            formData: {
                title: '',
                url: '',
                desc: '',
                sort: 0,
                pid: 0,
                type: '',
                attrs: '',
                system: '',
            },
            ruleValidate: {
                title: [
                    { required: true, message: 'The title cannot be empty', trigger: 'blur' }
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
        handleSubmit(name) {
            this.$refs[name].validate((valid) => {
                if (valid) {
                    this.subDisable = true;
                    if (this.menuId <= 0) {
                        let params = {
                            title: this.formData.title,
                            sys: this.formData.system,
                            url: this.formData.url,
                            type: this.formData.type,
                            sort: parseInt(this.formData.sort),
                            pid: parseInt(this.formData.pid),
                            attrs: this.formData.attrs,
                            desc: this.formData.desc
                        }
                        this.$axios.post(this.$API.PERMISSION_MENU_ADD, params)
                            .then(response => {
                                console.log(response);
                                if (response.data && response.data.rc == '0') {
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

                    } else {
                        let params = {
                            id: parseInt(this.menuId),
                            title: this.formData.title,
                            url: this.formData.url,
                            type: this.formData.type,
                            sort: parseInt(this.formData.sort),
                            pid: parseInt(this.formData.pid),
                            attrs: this.formData.attrs,
                            desc: this.formData.desc
                        }
                        this.$axios.post(this.$API.PERMISSION_MENU_EDIT, params)
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
                this.menuId = id;
                this.breadtitle = '菜单编辑';
                this.disableSystem = true;
                this.disableMenu = true;
                this.subDisable = true;

                let params = {
                    id: parseInt(id)
                }
                this.$axios.post(this.$API.PERMISSION_MENU_SHOW, params)
                    .then(response => {
                        console.log(response);
                        if (response.data && response.data.rc == '0') {

                            this.formData.title = response.data.title;
                            this.formData.url = response.data.url;
                            this.formData.system = response.data.system;
                            this.formData.desc = response.data.desc;
                            this.formData.type = response.data.type;
                            this.formData.sort = response.data.sort;
                            this.formData.pid = response.data.pid;
                            this.formData.attrs = response.data.attrs;

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

