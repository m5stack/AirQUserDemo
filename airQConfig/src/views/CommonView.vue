<script setup>


</script>

<template>
  <div class="bg-gray-50 min-h-screen flex flex-col">
    <nav class="sticky top-0 z-40 bg-blue-600">
      <div class="mx-auto max-w-6xl px-4 sm:px-6 lg:px-8">
        <div class="flex h-16 items-center justify-between">
          <div class="flex items-center text-white">
            <div class="flex-shrink-0">
              <h2 class="p-4 text-4xl sm:text-4xl font-semibold text-center">AirQ</h2>
            </div>
          </div>
        </div>
      </div>
    </nav>

    <div class="mx-auto w-full max-w-5xl flex-grow">
      <RouterView />
    </div>

    <div class="p-2 text-center text-gray-600 text-sm">
      Copyright &copy;2023 M5Stack
    </div>
  </div>
</template>