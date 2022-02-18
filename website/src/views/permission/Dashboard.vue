<template>
    <div>
    <Row style="background:#eee;padding:20px">
        <Col span="11">
            dashboard
        </Col>
    </Row>
    </div>

</template>
<script>
export default {
     data () {
            return {
                time1: (new Date()).getTime() - 60 * 3 * 1000,
                time2: (new Date()).getTime() - 86400 * 3 * 1000
            }
        }
}
</script>
