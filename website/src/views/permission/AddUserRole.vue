<template>
    <div>
        <Breadcrumb style="margin:10px">
            <BreadcrumbItem :to="{name:'dashboard'}">Home</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'accountlist'}">用户列表</BreadcrumbItem>
            <BreadcrumbItem :to="{name:'userroles', params:{ id:userid}}">用户角色列表</BreadcrumbItem>
            <BreadcrumbItem>添加角色</BreadcrumbItem>
        </Breadcrumb>

        <div class="content">
                <Card>
                  <Form ref="formData" :model="formData" :rules="ruleValidate" :label-width="80" style="width: 500px">
                          <FormItem label="所属系统">
                            <SystemListSelect :value.sync="system"></SystemListSelect>
                        </FormItem>
                        <FormItem label="角色" prop="roleId">
                             <Select v-model="formData.roleId" placeholder="角色" >
                                <Option v-for="item in formData.roleList" :value="item.id" :key="item.id">{{ item.name }}</Option>
                            </Select>
                        </FormItem>
                        <FormItem label="数据规则" prop="dataRule" >
                             <AutoComplete
                                v-model="formData.dataRule"
                                placeholder="数据规则"
                                style="width:300px">
                                <Option v-for="item in ruleList" :value="item.value" :key="item.value">{{ item.title }}</Option>
                            </AutoComplete>
                        </FormItem>

                        <FormItem label="数据" prop="dataList">
                            <Input v-model="formData.dataList" type="textarea" :autosize="{minRows: 2,maxRows: 5}" placeholder="全部匹配请填*, 其他每行一条 ..."></Input>
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
                userid:0,
                subDisable: false,
                ruleList:[
                    {
                        title : "所有",
                        value : "all"
                    },
                    {
                        title : "根据APPID",
                        value : "appId"
                    }
                ],
                system:'',
                formData: {
                    roleId: '',
                    system: '',
                    dataRule:'all',
                    dataList: '',
                    systemlist:[
                    ],
                    roleList:[]
                },
                ruleValidate: {
                    roleId: [
                        { required: true, type: 'number', message: 'select role', trigger: 'change' }
                    ]
                }
            }
        },
        watch: {
            system: function() {
                this.queryRoleList();
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
                        let params = {
                            userId:parseInt(this.userid),
                            sys:this.system,
                            roleId:this.formData.roleId,
                            dataRule:this.formData.dataRule,
                        }
                        if(this.formData.dataList) {
                            params.data = this.formData.dataList.split("\n");
                        }
                        this.$axios
                             .post(this.$API.PERMISSION_USERROLE_ADD, params)
                             .then(response => {
                                console.log(response);
                                if(response.data && response.data.rc == '0') {
                                    this.$Message.success('成功!');
                                } else {
                                    this.$Message.error('添加失败，请确认权限!');
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
            queryData() {
                this.userid = this.$route.params.id;
                this.fetchSysList();
                console.log(this.userid);
            },
            querytest() {

            },
            queryRoleList() {
                let params = {
                        sys:this.system,
                        size: 500,
                        type:1
                    };
                this.$axios
                     .post(this.$API.PERMISSION_ROLE_LIST, params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {
                            this.formData.roleList = response.data.list;
                        }
                    });
            },
            fetchSysList () {
                let params = {
                        size: 100
                    }
                this.$axios
                     .post(this.$API.PERMISSION_SYSTEM_LIST, params)
                     .then(response => {
                        console.log(response);
                        if(response.data && response.data.rc == '0') {
                            if(response.data.list) {
                                for(let i in response.data.list) {
                                    let name = response.data.list[i].name;
                                    let desc = response.data.list[i].desc;
                                    this.formData.systemlist.push({
                                        value: name,
                                        label: desc
                                    });
                                }
                            }
                        }
                    });
            }
        }
    }
</script>

