<script setup>
import { computed } from 'vue';

const props = defineProps(['display', 'text', 'type'])

const typeClass = computed(() => {
  return {
    'alert': true,
    'alert-success': props.type === 'success',
    'alert-error': props.type === 'error'
  }
})

</script>

<template>
  <div v-show="display" class="fixed top-20 left-1/2 transform -translate-x-1/2 w-5/6 sm:max-w-sm p-2 rounded-lg z-50" :class="typeClass">
    <svg v-if="type === 'error'" xmlns="http://www.w3.org/2000/svg" class="stroke-current shrink-0 h-6 w-6" fill="none" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>
    <svg v-if="type === 'success'" xmlns="http://www.w3.org/2000/svg" class="stroke-current shrink-0 h-6 w-6" fill="none" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>
    <span>{{ text }}</span>
  </div>
</template>