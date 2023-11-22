import { createRouter, createWebHashHistory, createWebHistory } from 'vue-router'
import WifiConfigView from '@/views/WifiConfigView.vue'

const router = createRouter({
  // history: createWebHistory(import.meta.env.BASE_URL),
  history: createWebHashHistory(),
  routes: [
    {
      path: '/',
      name: 'wifi',
      component: WifiConfigView
    },
    {
      path: '/device',
      name: 'device',
      component: () => import('@/views/CommonView.vue'),
      children: [
        {
          path: 'info',
          name: 'deviceInfo',
          component: () => import('@/views/InfoView.vue'),
        },
      ]
    },
  ]
})

export default router
