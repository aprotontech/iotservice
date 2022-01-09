<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem to="/dashboard">Home</BreadcrumbItem>
            <BreadcrumbItem to='/apis'>API列表</BreadcrumbItem>
            <BreadcrumbItem>{{breadtitle}}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
                <Card>
                  <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                        <FormItem label="所属系统" prop="system">
                            <SystemListSelect :value.sync="formData.system" :disableEdit.sync="disableEdit"></SystemListSelect>
                        </FormItem>
                        <FormItem label="权限标示" prop="permission" >
                            <Input v-model="formData.permission" placeholder="example：ota_view" :disabled="disableEdit"></Input>
                        </FormItem>
                        <FormItem label="请求地址" prop="uri" >
                            <Input v-model="formData.uri" placeholder="example：/test" ></Input>
                        </FormItem>
                        <FormItem label="匹配方法" prop="method">
                         <Select v-model="formData.type" placeholder="匹配方法">
                            <Option v-for="item in formData.mtypes" :value="item.value" :key="item.value">{{ item.label }}</Option>
                        </Select>
                        </FormItem>
                        <FormItem label="请求方法" prop="method">
                         <Select v-model="formData.method" placeholder="请求方法">
                            <Option v-for="item in formData.methods" :value="item.value" :key="item.value">{{ item.label }}</Option>
                        </Select>
                        </FormItem>
                        <FormItem label="请求域名" prop="domain" >
                            <Input v-model="formData.domain" placeholder="example：api.aproton.tech" ></Input>
                        </FormItem>
                        <FormItem label="服务接口地址" prop="serviceUrl" >
                            <Input v-model="formData.serviceUrl" placeholder="example：http://api.aproton.tech/test"></Input>
                        </FormItem>
                        <FormItem label="服务接口超时时间" prop="timeout" >
                            <Input v-model="formData.timeout" placeholder="单位秒"></Input>
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
                breadtitle:'API添加',
                disableEdit: false,
                subDisable:false,
                id:0,
                formData: {
                    permission: '',
                    system: '',
                    domain:'',
                    uri:'',
                    type: '0',
                    method:'ANY',
                    timeout: 3,
                    serviceUrl:'',
                    systemlist:[
                        {
                            value:'aproton',
                            label:'aproton',
                        },
                        {
                            value:'dmin',
                            label:'permission admin'
                        }
                    ],
                    mtypes: [
                        {
                            value: '0',
                            label: '完全匹配'
                        },
                        {
                            value: '1',
                            label: '前缀匹配'
                        }
                    ],
                    methods : [
                        {
                            value:'ANY',
                            label:'ANY'
                        },
                        {
                            value:'POST',
                            label:'POST'
                        },
                        {
                            value:'GET',
                            label:'GET'
                        },
                        {
                            value:'PUT',
                            label:'PUT'
                        },
                        {
                            value:'DELETE',
                            label:'DELETE'
                        }
                    ]
                },
                ruleValidate: {
                    permission: [
                        { required: true, message: 'The permission cannot be empty', trigger: 'blur' }
                    ],
                    system: [
                        { required: true, message: 'Please select the system', trigger: 'change' }
                    ],
                    domain: [
                        { required: true, message: 'The domain cannot be empty', trigger: 'blur' }
                    ],
                    uri: [
                        { required: true, message: 'The uri cannot be empty', trigger: 'blur' }
                    ],
                    method: [
                        { required: true, message: 'The method cannot be empty', trigger: 'change' }
                    ],
                    timeout: [
                        { type: 'number', message: 'The timeout must be number', trigger: 'blur', transform(value) {
                                    return Number(value);
                        }}
                    ],
                    type: [
                        { required: true, message: 'The match type cannot be empty', trigger: 'change' }
                    ],
                    serviceUrl: [
                        { required: true, message: 'The service url cannot be empty', trigger: 'blur' }
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
                        if(this.id <= 0) {
                             let params = {
                                permission:this.formData.permission,
                                sys:this.formData.system,
                                domain:this.formData.domain,
                                uri:this.formData.uri,
                                type: this.formData.type,
                                method:this.formData.method,
                                timeout:parseFloat(this.formData.timeout),
                                serviceUrl:this.formData.serviceUrl
                            }
                             this.$axios
                                 .post('/admin/api/add', params)
                                 .then(response => {
                                    console.log(response);
                                    if(response.data && response.data.rc == '0') {
                                        this.$Message.success('成功!');
                                    } else if (response.data.rc == '-3003') {
                                        this.$Message.error('未登陆!');
                                    } else if (response.data.rc == '-4001') {
                                        this.$Message.error('无权访问!');
                                    } else if (response.data.rc == '-4') {
                                        this.$Message.error('权限不存在!');
                                    } else if (response.data.rc == '-5004') {
                                        this.$Message.error('uri已存在!');
                                    } else {
                                        this.$Message.error('失败!');
                                    }
                                    this.subDisable = false;
                                });


                        } else {
                            let params = {
                                id:parseInt(this.id),
                                domain:this.formData.domain,
                                uri:this.formData.uri,
                                type: this.formData.type,
                                method:this.formData.method,
                                timeout:parseFloat(this.formData.timeout),
                                serviceUrl:this.formData.serviceUrl
                            }
                             this.$axios
                                 .post('/admin/api/edit', params)
                                 .then(response => {
                                    console.log(response);
                                    if(response.data && response.data.rc == '0') {
                                        this.$Message.success('成功!');
                                    } else if (response.data.rc == '-3003') {
                                        this.$Message.error('未登陆!');
                                    } else if (response.data.rc == '-4001') {
                                        this.$Message.error('无权访问!');
                                    } else if (response.data.rc == '-4') {
                                        this.$Message.error('权限不存在!');
                                    } else if (response.data.rc == '-5004') {
                                        this.$Message.error('uri已存在!');
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
                this.formData.system = this.$route.query.system;
                this.formData.permission = this.$route.query.p;
                if(id > 0) {
                    this.id = id;
                    this.breadtitle = 'API编辑';
                    this.disableEdit = true;
                    this.subDisable = true;

                     let params = {
                        id:parseInt(id)
                    }
                    this.$axios
                     .post('/admin/api/show', params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {

                            this.formData.permission = response.data.permission;
                            this.formData.system = response.data.system;
                            this.formData.domain = response.data.request_domain;
                            this.formData.uri = response.data.request_uri;
                            this.formData.method = response.data.request_method;
                            this.formData.timeout = response.data.timeout;
                            this.formData.serviceUrl = response.data.service_url;
                            this.formData.type = response.data.type + '';

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

