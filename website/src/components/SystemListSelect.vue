<template>
    <Select v-model="tmp_selected" placeholder="所属系统"  :disabled="disableEdit" clearable>
        <Option v-for="item in systemlist" :value="item.value" :key="item.value">{{ item.label }}</Option>
    </Select>
</template>
<script>
export default {
    name: 'SystemListSelect',
    props: ['value','disableEdit'],
   
    data: function () {
        return {
            systemlist : [],
            tmp_selected: this.value
        }
    },
    created() {
        this.fetchSysList();
    },
    watch: {
      tmp_selected (val) {
        this.$emit('update:value', val)
      }
    },
    mounted() {
    },
    methods: {
        onChange(val){
        //    this.$emit('update:value', val)
        },
        fetchSysList () {
            let params = {
                page: 1,
                size: 100
            }
            this.$axios
                 .post(this.$API.PERMISSION_SYSTEM_LIST, params)
                 .then(response => {
                    if(response.data && response.data.rc == '0') {
                        if(response.data.list) {
                            for(let i in response.data.list) {
                                let name = response.data.list[i].name;
                                let desc = response.data.list[i].desc;
                                this.systemlist.push({
                                    value: name,
                                    label: name
                                });
                            }
                        }
                    }
                    this.tmp_selected = this.value
                });
            }
    }
}
</script>

