<template>
  <v-main>
    <v-container class="py-8 px-6" fluid>
      <v-row>
        <v-col v-for="{ icon, id, name, description, image } in techniques" cols="4" :key="name">
          <v-card class="mx-auto">
            <v-img
              :src="image"
              height="220px"
            ></v-img>

            <v-card-title><v-icon color="secondary" left>{{ icon }}</v-icon> {{ name }}</v-card-title>
            <v-card-subtitle>{{ description }}</v-card-subtitle>

            <v-card-actions>
              <v-btn color="primary" text :to="`/arsenal/${id}`">Learn More</v-btn>
            </v-card-actions>
          </v-card>
        </v-col>
      </v-row>
    </v-container>
  </v-main>
</template>

<script>
import TechniquesService from '@/services/techniques';

export default {
  name: "About",

  title: "Sparta - Arsenal",

  data: () => ({
    techniques: [],
  }),

  mounted() {
    this.techniques = TechniquesService.getAllTechniques();
  },
};
</script>
