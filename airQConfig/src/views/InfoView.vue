<script setup>
import { ref, onMounted } from 'vue';
import { deviceStore } from '@/stores/store.js';
import QRCode from 'qrcode'
import TipComponent from '../components/TipComponent.vue';

let tipSuccessTimer
let tipErrorTimer

const storeDevice = deviceStore()
const monitorLink = ref()
const qrcode = ref()
const isLoading = ref(true)
const isCopying = ref(false)
const tipSuccessText = ref('')
const tipErrorText = ref('')
const tipSuccess = ref(false)
const tipError = ref(false)

onMounted(() => {
  monitorLink.value = `https://airq.m5stack.com/${storeDevice.mac}`
  generateQrcode()
  isLoading.value = false
  wifiControl()
})

const wifiControl = () => {
  fetch('/api/v1/ap_control', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      "status": true
    })
  })
    .then(res => {
      if (res.status === 201 || res.status === 200) {
        return res.json()
      } else {
        throw new Error()
      }
    })
    .then(res => {
      return res
    })
    .catch(err => {
      console.log(err)
    })
}

const generateQrcode = () => {
  const url = monitorLink.value
  QRCode.toCanvas(qrcode.value, url, { width: 200, height: 200, margin: 1  }, function (error) {
    if (error) console.error(error)
  })
}

const handleCopy = () => {
  if (isCopying.value) return
  isCopying.value = true
  if (monitorLink.value) {
    const text = monitorLink.value
    copyTextToClipboard(text)
    tipShow('success', 'Copied')
  } else {
    tipShow('error', 'Copy failed.')
  }
}

const copyTextToClipboard = text => {
  const textarea = document.createElement('textarea');
  textarea.value = text;
  textarea.setAttribute('readonly', '');
  textarea.style.position = 'absolute';
  textarea.style.left = '-9999px';
  document.body.appendChild(textarea);

  textarea.select();
  document.execCommand('copy');

  document.body.removeChild(textarea);
}

const tipShow = (type, text) => {
  if (type === 'success') {
    clearTimeout(tipSuccessTimer)
    tipSuccessText.value = text
    tipSuccess.value = true
    setTimeout(() => {
      tipSuccess.value = false
      isCopying.value = false
    }, 1500)
  } else {
    clearTimeout(tipErrorTimer)
    tipErrorText.value = text
    tipError.value = true
    setTimeout(() => {
      tipError.value = false
      isCopying.value = false
    }, 1500)
  }
}

</script>

<template>
  <div class="p-3">
    <div class="rounded-lg shadow-md bg-white my-4 mx-2 text-gray-900">
      <div class="px-4 py-3 sm:px-6 text-xl text-gray-600 bg-slate-300 rounded-t-lg sm:font-semibold">
        AirQ Monitor
      </div>
      <div class="py-2 px-4 sm:px-6 flex flex-col bg-slate-100 text-gray-600 last:rounded-b-lg">
        
        <div class="flex justify-center">
          <div v-show="isLoading" class="w-[220px] h-[220px] flex items-center justify-center">
            <span class="loading loading-spinner loading-lg bg-blue-600 text-lg"></span>
          </div>
          <canvas v-show="!isLoading" ref="qrcode" class="w-[220px] h-[220px]"></canvas>
        </div>
        <div v-show="!isLoading" class="flex flex-col sm:flex-row justify-center content-center">
          <a class="break-all underline mr-2" :href="monitorLink">{{ monitorLink }}</a>
          <button
            class="rounded-full px-2 py-2 sm:py-0 my-2 text-white bg-indigo-600 hover:bg-indigo-500"
            @click="handleCopy"
          >
            COPY
          </button>
        </div>
        <p v-show="!isLoading" class="text-red-500 flex justify-center">Notice: please disconnect from the AirQ AP to access this address</p>
      </div>
    </div>
    <TipComponent :text="tipSuccessText" type="success" :display="tipSuccess" />
    <TipComponent :text="tipErrorText" type="error" :display="tipError" />
  </div>
</template>