import { defineStore } from "pinia";

export const deviceStore = defineStore('device', {
  state: () => {
    return {
      mac: '',
      token: '',
      ip: ''
    }
  },
  persist: {
    enabled: true
  }
})