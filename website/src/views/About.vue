<template>
  <div class="about">
    <h1>This is an about page {{ msgdata }} {{ this.$store.state.username }}</h1>

  </div>
</template>

<script>

export default {
  name: 'about',
  data() {
    return {
      msgdata: "",
      ok: false
    }
  },
  created() {
    this.$axios.get('/version').then(response => {
      this.msgdata = response.data.version;
    });

  },
}
</script>
