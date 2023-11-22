<script setup>
import { computed, onMounted, ref } from 'vue';
import { useRouter } from 'vue-router';
import TipComponent from '../components/TipComponent.vue';
import { deviceStore } from '../stores/store';

const timezones = [
  'GMT0',
  'GMT-1',
  'GMT-2',
  'GMT-3',
  'GMT-4',
  'GMT-5',
  'GMT-6',
  'GMT-7',
  'GMT-8',
  'GMT-9',
  'GMT-10',
  'GMT-11',
  'GMT-12',
  'GMT+1',
  'GMT+2',
  'GMT+3',
  'GMT+4',
  'GMT+5',
  'GMT+6',
  'GMT+7',
  'GMT+8',
  'GMT+9',
  'GMT+10',
  'GMT+11',
  'GMT+12',
]

const intervals = [
  { 'name': '10s', value: 10 },
  { 'name': '30s', value: 30 },
  { 'name': '60s', value: 60 },
  { 'name': '5min', value: 300 },
  { 'name': '10min', value: 600 },
  { 'name': '30min', value: 1800 },
  { 'name': '60min', value: 3600 },
]

const states = [
  { 'name': 'ON', value: true },
  { 'name': 'OFF', value: false }
]

const data = ref({
  "config": {
    "rtc": {
      "sleep_interval": 60
    },
    "ntp": {
      "server_0": "pool.ntp.org",
      "server_1": "time.nist.gov",
      "tz": "GMT+8",
    },
    "buzzer": {
      "mute": true
    }
  }
})

let alertTimeout
let getWifiStatusInterval
let connectTimeOut = 12000 // ms

const storeDevice = deviceStore()
const router = useRouter()
const isConnect = ref(false)
const ssid = ref('')
const password = ref('')
const showPassword = ref(false)
const ssidTip = ref(false)
const pwdTip = ref(false)
const activeItem = ref()
const isConnectFail = ref(false)
// const factoryState = ref(true)
const wifilist = ref([])
const pwdInput = ref()
const tipText = ref('')
const showDropdown = ref(false)
const intervalTip = ref(false)
const intervalTipText = ref('')
const intervalInput = ref()

const typePwd = computed(() => {
  return showPassword.value ? 'text' : 'password'
})

onMounted(() => {
  getWlan()
  console.log(connectTimeOut)
})

const itemActive = i => {
  const isActive = activeItem.value === i
  return {
    'bg-indigo-600': isActive,
    'text-white': isActive,
    'text-black': !isActive,
    'bg-slate-100': !isActive
  }
}

const getWlan = () => {
  fetch('/api/v1/wifi_list')
    .then(res => {
      if (res.status === 200) {
        return res.json()
      } else {
        throw new Error()
      }
    })
    .then(res => {
      wifilist.value = res.ap_list
    })
    .catch(err => {
      console.log(err)
    })
}

const checkSSIDInput = () => {
  if (ssid.value.trim() === '') {
    ssidTip.value = true
  } else {
    ssidTip.value = false
  }
}

const checkPWDInput = () => {
  if (password.value.trim() === '') {
    pwdTip.value = true
  } else {
    pwdTip.value = false
  }
}

const handleConnect = () => {
  if (ssid.value.trim() === '' || password.value.trim() === '') {
    checkPWDInput()
    checkSSIDInput()
    return
  }

  isConnect.value = true

  handlePostConfig()

  fetch('/api/v1/wifi_connect', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      "wifi": {
        "ssid": ssid.value,
        "password": password.value
      }
    })
  })
    .then(res => {
      if (res.status === 200 || res.status === 201) {
        return res.json()
      } else {
        throw new Error()
      }
    })
    .then(res => {
      clearInterval(getWifiStatusInterval)
      getWifiStatusInterval = setInterval(() => {
        if (connectTimeOut === 0) {
          connectTimeOut = 12000
          clearInterval(getWifiStatusInterval)
          tipText.value = 'Connect fail.'
          isConnectFail.value = true
          clearTimeout(alertTimeout)
          alertTimeout = setTimeout(() => isConnectFail.value = false, 1500)
          isConnect.value = false
        } else {
          connectTimeOut -= 500
          getWifiStatus()
        }
      }, 500)
    })
    .catch(err => {
      tipText.value = 'Connect fail.'
      isConnectFail.value = true
      clearTimeout(alertTimeout)
      alertTimeout = setTimeout(() => isConnectFail.value = false, 1500)
      console.log(err)
    })
}

const getWifiStatus = () => {
  fetch('/api/v1/wifi_status')
    .then(res => {
      if (res.status === 200) {
        return res.json()
      } else {
        throw new Error()
      }
    })
    .then(res => {
      if (res.psk_status) {
        if (res.status) {
          clearInterval(getWifiStatusInterval)
          storeDevice.mac = res.mac.replaceAll(':', '')
          router.push({ name: 'deviceInfo' })
        }
      } else {
        tipText.value = 'Incorrect password.'
        isConnectFail.value = true
        clearTimeout(alertTimeout)
        alertTimeout = setTimeout(() => isConnectFail.value = false, 1500)
        clearInterval(getWifiStatusInterval)
        isConnect.value = false
      }
    })
    .catch(err => {
      tipText.value = 'Connect fail.'
      isConnectFail.value = true
      clearTimeout(alertTimeout)
      alertTimeout = setTimeout(() => isConnectFail.value = false, 1500)
      clearInterval(getWifiStatusInterval)
      isConnect.value = false
      console.log(err)
    })
}

const handlePostConfig = () => {
  let d = JSON.parse(JSON.stringify(data.value))
  fetch('/api/v1/config', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(d)
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

const handleSelectWifi = (wifi, index) => {
  ssid.value = wifi
  activeItem.value = index
  pwdInput.value.focus()
  ssidTip.value = false
}

const handleIntervalInput = (e) => {
  const isNumber = /^\d+$/g.test(e.target.value)
  if (!isNumber) {
    intervalTipText.value = 'please enter number'
    intervalTip.value = true
  } else if (Number(e.target.value) > 86400) {
    intervalTipText.value = 'is over than 84600s (24h)'
    intervalTip.value = true
  } else {
    intervalTip.value = false
  }

  data.value.config.rtc.sleep_interval = Number(e.target.value)
}

const closeDropdown = (event) => {
  if (!intervalInput.value.contains(event.target)) {
    showDropdown.value = false;
    document.removeEventListener('click', closeDropdown);
  }
}

const selectItem = (value) => {
  data.value.config.rtc.sleep_interval = value;
  showDropdown.value = false;
  intervalTip.value = false
}

const handleToggleDropdown = () => {
  showDropdown.value = !showDropdown.value
  if (showDropdown.value) {
    document.addEventListener('click', closeDropdown);
  } else {
    document.removeEventListener('click', closeDropdown);
  }
}

</script>

<template>
  <div class="flex min-h-screen flex-col justify-center items-center sm:justify-start px-6 lg:px-8 bg-white">
    <div class="w-full py-4 sm:max-w-sm">
      <h2 class="text-center text-4xl font-semibold leading-9 tracking-tight text-gray-900">
        AirQ WiFi Setup
      </h2>
    </div>

    <div class="flex-grow w-full">
      <div class="w-full mx-auto mb-2 sm:max-w-sm rounded-md border-slate-300">
        <h2 class="text-xl p-3 rounded-t-md bg-slate-300">WiFi</h2>
        <ul v-if="wifilist.length">
          <li
            class="cursor-pointer p-3 border-b border-b-slate-300 hover:bg-indigo-600 hover:text-white text-md last:rounded-b-md last:border-none"
            v-for="(item, i) in wifilist" :class="itemActive(i)" @click="handleSelectWifi(item, i)">
            {{ item }}
          </li>
        </ul>
        <div v-else class="p-3 bg-slate-100 text-md rounded-b-md">
          None
        </div>
      </div>

      <div class="w-full mx-auto sm:max-w-sm mb-4">
        <form class="space-y-4 p-3 bg-slate-100 rounded-md">
          <div class="">
            <label for="ssid" class="block text-md text-gray-900">
              WiFi
            </label>
            <div class="mt-2">
              <input id="ssid" type="text"
                class="block w-full rounded-md border border-gray-600 py-1.5 px-2 text-gray-900 shadow-sm sm:text-sm sm:leading-6"
                v-model="ssid" @blur="checkSSIDInput">
              <p v-show="ssidTip" class="text-red-500 text-sm">Please enter the WiFi</p>
            </div>
          </div>
          <div>
            <div class="flex items-center justify-between">
              <label for="password" class="block text-md text-gray-900">
                Password
              </label>
            </div>
            <div class="mt-2">
              <input ref="pwdInput" id="password" :type="typePwd"
                class="block w-full rounded-md border border-gray-600 py-1.5 px-2 text-gray-900 shadow-sm sm:text-sm sm:leading-6"
                v-model="password" @blur="checkPWDInput">
              <p v-show="pwdTip" class="text-red-500 text-sm">Please enter the password.</p>
              <div class="mt-2 flex items-center">
                <input type="checkbox" id="showpwd" class="checkbox checkbox-xs mr-2" v-model="showPassword" />
                <label for="showpwd" class="text-sm text-gray-900">Show Password</label>
              </div>
            </div>
          </div>
        </form>
      </div>

      <div class="rounded-lg mx-auto sm:max-w-sm bg-slate-100 my-4">
        <div class="bg-slate-300 rounded-t-lg p-3 sm:px-6 text-xl sm:font-semibold text-gray-600">
          Power Config
        </div>
        <div class="p-4 text-gray-600">
          <div class="my-2" ref="intervalInput">
            <label for="" class="inline-block w-[198px] ">Wake-up Interval (s)</label>
            <div class="relative w-42 h-[34px]">
              <input class="border rounded p-1 w-full" @input="e => handleIntervalInput(e)"
                :value="data.config.rtc.sleep_interval" />
              <i class="interval-input" @click="handleToggleDropdown"></i>
              <p v-show="intervalTip" class="text-red-500 text-sm pl-2">{{ intervalTipText }}</p>
              <ul v-if="showDropdown" class="absolute top-[30px] w-full rounded-t-md border bg-white cursor-pointer ">
                <li class="hover:bg-slate-100 p-2" v-for="item in intervals" :key="item.name"
                  @click="selectItem(item.value)">
                  {{ item.name }}
                </li>
              </ul>
            </div>
          </div>
        </div>
      </div>
      <div class="rounded-lg mx-auto sm:max-w-sm bg-slate-100 my-4">
        <div class="bg-slate-300 rounded-t-lg p-3 sm:px-6 text-xl sm:font-semibold text-gray-600">
          NTP Config
        </div>
        <div class="p-4 text-gray-600">
          <div>
            <label for="server_0" class="inline-block w-48 mr-2">Server 0</label>
            <input type="text" id="server_0" class="border rounded p-1 w-full" v-model="data.config.ntp.server_0">
          </div>
          <div class="my-2">
            <label for="" class="inline-block w-48 mr-2">Server 1</label>
            <input type="text" class="border rounded p-1 w-full" v-model="data.config.ntp.server_1">
          </div>
          <div class="my-2">
            <label for="time_zone" class="inline-block w-48 mr-2">Time Zone</label>
            <select id="time_zone" class="select border bg-white rounded p-1 h-8 w-full" v-model="data.config.ntp.tz">
              <option v-for="time in timezones" :value="time">
                {{ time }}
              </option>
            </select>
          </div>
        </div>
      </div>
      <div class="rounded-lg mx-auto sm:max-w-sm bg-slate-100 my-4">
        <div class="bg-slate-300 rounded-t-lg p-3 sm:px-6 text-xl sm:font-semibold text-gray-600">
          Buzzer Config
        </div>
        <div class="p-4 text-gray-600">
          <div class="my-2">
            <label for="buzzer" class="inline-block w-48 mr-2">State</label>
            <select id="buzzer" class="select border bg-white rounded p-1 h-8 w-full" v-model="data.config.buzzer.mute">
              <option v-for="state in states" :value="state.value">
                {{ state.name }}
              </option>
            </select>
          </div>
        </div>
      </div>


      <div class="w-full mx-auto sm:max-w-sm mb-4">
        <button class="w-full btn rounded-full text-sm font-semibold leading-6 shadow-sm" :disabled="isConnect"
          @click.prevent="handleConnect">
          <span v-show="isConnect" class="loading loading-spinner"></span>
          Connect
        </button>
      </div>
    </div>
    <div class="py-2 text-center text-gray-600 text-sm">
      Copyright &copy;2023 M5Stack
    </div>

    <TipComponent :text="tipText" type="error" :display="isConnectFail" />
  </div>
</template>

<style scoped>
.input-group {
  position: relative;
}

.interval-input {
  width: 34px;
  height: 34px;
  display: block;
  position: absolute;
  right: 0px;
  top: 0px;
  cursor: pointer;
  background-image: linear-gradient(45deg, transparent 50%, currentColor 50%), linear-gradient(135deg, currentColor 50%, transparent 50%);
  background-position: calc(100% - 20px) calc(1px + 50%), calc(100% - 16.1px) calc(1px + 50%);
  background-size: 4px 4px, 4px 4px;
  background-repeat: no-repeat;
}

.dropdown {
  position: absolute;
  top: 30px;
  border-radius: 2px;
  background-color: #fff;
  list-style-type: none;
  padding: 0;
  border: 1px solid #ccc;
  z-index: 99;
}

.dropdown li {
  padding: 5px 10px;
  cursor: pointer;
}

.dropdown li:hover {
  background-color: rgb(37 99 235 / var(--tw-bg-opacity));
}

.dropdown li:hover {
  background-color: #f0f0f0;
}
</style>