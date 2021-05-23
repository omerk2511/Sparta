<template>
  <v-app id="inspire">
    <v-system-bar app color="primary">
      <v-spacer></v-spacer>

      <v-icon color="accent">mdi-laptop</v-icon>

      <v-icon color="accent">mdi-needle</v-icon>

      <v-icon color="accent">mdi-shield-bug</v-icon>
    </v-system-bar>

    <v-navigation-drawer
      v-model="drawer"
      app
    >
      <v-sheet
        color="grey lighten-4"
        class="pa-4"
      >
        <v-icon
          class="mb-4"
          color="primary"
          size="64"
        >mdi-virus</v-icon>

        <h1>Sparta</h1>
      </v-sheet>

      <v-divider></v-divider>

      <v-list>
        <v-list-item
          v-for="[icon, text, link] in links"
          :key="icon"
          link
          :to="link"
        >
          <v-list-item-icon>
            <v-icon>{{ icon }}</v-icon>
          </v-list-item-icon>

          <v-list-item-content>
            <v-list-item-title>{{ text }}</v-list-item-title>
          </v-list-item-content>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>

    <router-view></router-view>
  </v-app>
</template>

<script>
export default {
  name: 'App',

  title: 'Sparta',

  data: () => ({
    drawer: null,
    links: [
      ['mdi-inbox', 'Threats', '/'],
      ['mdi-book-open-blank-variant', 'Arsenal', '/arsenal'],
      ['mdi-account', 'About', '/about'],
    ],
  }),
};
</script>
