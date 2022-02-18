<template>
    <div class="layout">
        <Layout>
        
            <Content>
                <Breadcrumb :style="{margin: '100px 0'}">
                 
                </Breadcrumb>
                <Row type="flex" justify="center" class="code-row-bg">
                <Col span="8">
                <Card>
                    <p slot="title">管理后台登陆</p>
                        <Form ref="formCustom" :model="formCustom" :rules="ruleCustom" :label-width="80">
                        <FormItem label="账号" prop="account">
                            <Input type="text" v-model="formCustom.account"></Input>
                        </FormItem>
                        <FormItem label="密码" prop="passwd">
                            <Input type="password" v-model="formCustom.passwd"></Input>
                        </FormItem>
                        <FormItem>
                            <Button type="primary" @click="handleSubmit('formCustom')">登陆</Button>
                        </FormItem>
                    </Form>

                </Card>
                </Col>
                </Row>
            </Content>
           <Footer class="layout-footer-center">APROTON</Footer>
        </Layout>
    </div>
</template>

<script>
import VueCookie from 'vue-cookie';

export default {
  name: 'login',
  data () {
        const validateAccount = (rule, value, callback) => {
            if (value === '') {
                callback(new Error('Please enter your account(name or email)'));
            } else {
                callback();
            }
        };
         const validatePwd = (rule, value, callback) => {
            if (value === '') {
                callback(new Error('Please enter your password'));
            } else {
                callback();
            }
        };

            return {
                formCustom: {
                    account: '',
                    passwd: '',
                },
                ruleCustom: {
                    passwd: [
                        { validator: validatePwd, trigger: 'blur' }
                    ],
                    account: [
                        { validator: validateAccount, trigger: 'blur' }
                    ],
                }
            }
        },
        methods: {
            handleSubmit (name) {
                this.$refs[name].validate((valid) => {
                    if (valid) {
                        let params = {
                            account:this.formCustom.account,
                            password:this.formCustom.passwd
                        };
                        this.$axios
                         .post('/web/login', params)
                         .then(response => {
                            if(!response.data || response.data.rc != '0') {
                                alert('登陆失败，请检查用户名密码');
                            } else {
                                this.$router.push('/dashboard');
                            }
                        });

                    } else {
                        //this.$Message.error('Fail!');
                    }
                })
            },
            
        }
}
</script>


<style scoped>
.layout{
    border: 1px solid #d7dde4;
    background: #f5f7f9;
    position: relative;
    border-radius: 4px;
    overflow: hidden;
}
.layout-logo{
    width: 100px;
    height: 30px;
    background: #5b6270;
    border-radius: 3px;
    float: left;
    position: relative;
    top: 15px;
    left: 20px;
}
.layout-nav{
    width: 420px;
    margin: 0 auto;
    margin-right: 20px;
}
.layout-footer-center{
    text-align: center;
}
</style>
