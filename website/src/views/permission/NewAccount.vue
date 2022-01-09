<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'accountlist'}">用户列表</BreadcrumbItem>
            <BreadcrumbItem>{{breadtitle}}</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
                <Card>
                  <Form ref="formData" :model="formData" :rules="ruleValidate"  :label-width="80" style="width: 500px">
                        <FormItem label="邮箱" prop="email" >
                            <Input v-model="formData.email" placeholder="不能重复" :disabled="false"></Input>
                        </FormItem>

                        <FormItem label="用户名" prop="desc">
                            <Input v-model="formData.name" :disabled="false"></Input>
                        </FormItem>
                        <FormItem label="密码" prop="desc">
                            <Input v-model="formData.password" :disabled="false"></Input>
                            <Button @click="randPasswd()">随机生成</Button>
                        </FormItem>
                        <FormItem>
                            <Button type="primary" @click="handleSubmit('formData')" :disabled="subDisable">保存</Button>
                        </FormItem>
                    </Form>
                </Card>
        </div>

    </div>
</template>
<script>
  import {upDownState, ApiMessage, formatDate} from '@/utils/util'
export default {
        data () {
            return {
                id:0,
                breadtitle:'添加用户',
                disableEdit: false,
                subDisable: false,
                formData: {
                    name:'',
                    email: '',
                    password: '',
                },
                ruleValidate: {
                    name: [
                        { required: true, message: 'The name cannot be empty', trigger: 'blur' }
                    ],
                    email: [
                        { required: true, message: 'The Email cannot be empty', trigger: 'blur' }
                    ]
                }
            }
        },
        created() {
        },
        methods: {
            handleSubmit (name) {
                this.$refs[name].validate((valid) => {
                    if (valid) {
                        this.subDisable = true;
                        
                        let params = this.formData
                        this.$axios
                             .post(this.$API.PERMISSION_ACCOUNT_CREATE, params)
                             .then(response => {
                                console.log(response);
                                if(response.data && response.data.rc == '0') {
                                    this.$Message.success('成功!');
                                } else {
                                    this.$Message.error(response.data.err);
                                }
                                this.subDisable = false;
                            });
                    } else {
                        // this.$Message.error('Fail!');
                    }
                })
            },
            handleReset (name) {
                this.$refs[name].resetFields();
            },
            randPasswd() {
                var returnStr = "",
                   charStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
                for(var i=0; i<16; i++){
                  var index = Math.round(Math.random() * (charStr.length-1));
                  returnStr += charStr.substring(index,index+1);
                }
                this.formData.password = returnStr
            },
        }
    }
</script>

